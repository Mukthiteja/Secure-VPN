#include "vpn/vpn_server.h"

#include <Poco/Net/SecureServerSocket.h>
#include <Poco/Net/TCPServerParams.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/ConsoleCertificateHandler.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Buffer.h>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <Poco/Thread.h>
#include <Poco/Timespan.h>
#include <iostream>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/UUIDGenerator.h>
#include "vpn/tunnel.h"
#include "vpn/crypto.h"
#include "vpn/auth.h"

using Poco::Net::Context;
using Poco::Net::SecureServerSocket;
using Poco::Net::TCPServer;
using Poco::Net::TCPServerConnection;
using Poco::Net::TCPServerConnectionFactory;
using Poco::Net::TCPServerParams;
using Poco::Net::SSLManager;

namespace vpn {

class VpnServer::Connection : public TCPServerConnection {
public:
	Connection(const Poco::Net::StreamSocket& s, CredentialStore::Ptr store)
		: TCPServerConnection(s)
		, _store(std::move(store)) {}

	void run() override {
		try {
			Poco::Net::SecureStreamSocket secureSock(socket());
			vpn::Tunnel tunnel(secureSock);
			// Handshake
			auto serverSessionId = Poco::UUIDGenerator::defaultGenerator().createRandom().toString();
			std::string clientSessionId;
			std::vector<std::uint8_t> clientNonce, serverNonce, keySeed;
			tunnel.serverHandshake(serverSessionId, clientSessionId, clientNonce, serverNonce, keySeed);
			auto keys = vpn::deriveSessionKeys(keySeed, clientNonce, serverNonce);
			vpn::SessionCrypto sessionCrypto(keys.encKey, keys.macKey);
			Poco::Logger::get("VpnServer").information(Poco::format("Session established serverId=%s clientId=%s", serverSessionId, clientSessionId));

			// Authentication phase
			auto authCipher = tunnel.receiveAuth(std::chrono::milliseconds(10000));
			if (authCipher.empty()) {
				tunnel.sendAuthResult(false, "Authentication timeout");
				tunnel.sendClose();
				return;
			}

			std::string username;
			std::string password;
			try {
				auto authPlain = sessionCrypto.decrypt(authCipher);
				std::string authJson(authPlain.begin(), authPlain.end());
				Poco::JSON::Parser parser;
				auto result = parser.parse(authJson);
				auto obj = result.extract<Poco::JSON::Object::Ptr>();
				username = obj->getValue<std::string>("username");
				password = obj->getValue<std::string>("password");
			} catch (const std::exception& ex) {
				Poco::Logger::get("VpnServer").warning(Poco::format("Auth parse error: %s", ex.what()));
				tunnel.sendAuthResult(false, "Invalid auth payload");
				tunnel.sendClose();
				return;
			}

			if (!_store || !_store->verify(username, password)) {
				Poco::Logger::get("VpnServer").warning(Poco::format("Authentication failed for user %s", username));
				tunnel.sendAuthResult(false, "Authentication failed");
				tunnel.sendClose();
				return;
			}
			tunnel.sendAuthResult(true, "OK");
			Poco::Logger::get("VpnServer").information(Poco::format("User %s authenticated", username));

			// Main loop: handle DATA and HEARTBEAT
			for (;;) {
				// Prefer encrypted data
				auto enc = tunnel.receiveEncrypted(std::chrono::milliseconds(30000));
				if (!enc.empty()) {
					try {
						auto plain = sessionCrypto.decrypt(enc);
						// Echo plaintext back as encrypted
						auto resp = sessionCrypto.encrypt(plain);
						tunnel.sendEncrypted(resp);
					} catch (const std::exception& ex) {
						Poco::Logger::get("VpnServer").warning(Poco::format("Decrypt error: %s", ex.what()));
					}
					continue;
				}
				// Fallback to legacy DATA if present
				auto data = tunnel.receiveData(std::chrono::milliseconds(1));
				if (!data.empty()) {
					tunnel.sendData(data);
					continue;
				}
				// Check heartbeat
				if (tunnel.receiveHeartbeat(std::chrono::milliseconds(1))) {
					// Optionally respond with heartbeat for liveness symmetry
					tunnel.sendHeartbeat();
					continue;
				}
				// If no data and no heartbeat in the interval, loop continues until timeout
			}
		} catch (const std::exception& ex) {
			Poco::Logger::get("VpnServer").warning(Poco::format("Connection error: %s", ex.what()));
		}
	}

private:
	CredentialStore::Ptr _store;
};

class ConnectionFactory : public TCPServerConnectionFactory {
public:
	explicit ConnectionFactory(CredentialStore::Ptr store)
		: _store(std::move(store)) {}

	TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) override {
		return new VpnServer::Connection(socket, _store);
	}

private:
	CredentialStore::Ptr _store;
};

VpnServer::VpnServer(const ServerConfig& config)
	: _config(config) {
}

VpnServer::~VpnServer() {
	stop();
}

void VpnServer::start() {
	if (_running) return;

	// Configure SSL/TLS context
	_sslContext = std::make_shared<Context>(
		Context::SERVER_USE,
		_config.keyFile,
		_config.certFile,
		_config.caFile,
		Context::VERIFY_RELAXED,
		9, // verification depth
		true, // load default CA
		"ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"
	);
	if (_config.requireClientAuth) {
		_sslContext->requireClientVerification(true);
	}

	// Setup SSL manager with simple console handlers (placeholder)
	Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> certHandler = new Poco::Net::ConsoleCertificateHandler(true);
	Poco::SharedPtr<Poco::Net::PrivateKeyPassphraseHandler> pkeyHandler = new Poco::Net::KeyConsoleHandler(false);
	SSLManager::instance().initializeServer(pkeyHandler, certHandler, _sslContext.get());

	_credentialStore = CredentialStore::loadFromFile(_config.credentialFile);

	SecureServerSocket svs(Poco::Net::SocketAddress(_config.address, _config.port), 64, _sslContext.get());
	auto params = new TCPServerParams;
	params->setMaxThreads(16);
	params->setMaxQueued(64);
	params->setThreadIdleTime(Poco::Timespan(10, 0));

	_tcpServer = std::make_unique<TCPServer>(new ConnectionFactory(_credentialStore), svs, params);
	_tcpServer->start();
	_running = true;
	Poco::Logger::get("VpnServer").information("VPN server started");
}

void VpnServer::stop() {
	if (!_running) return;
	_tcpServer->stop();
	_tcpServer.reset();
	_credentialStore.reset();
	_running = false;
	Poco::Logger::get("VpnServer").information("VPN server stopped");
}

} // namespace vpn


