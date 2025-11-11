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
	explicit Connection(const Poco::Net::StreamSocket& s)
		: TCPServerConnection(s) {}

	void run() override {
		try {
			Poco::Net::SocketStream stream(socket());
			stream << "OK VPN-HELLO\n";
			stream.flush();
			// Placeholder: Echo loop for initial connectivity validation
			char buffer[4096];
			int n = 0;
			while ((n = socket().receiveBytes(buffer, sizeof(buffer))) > 0) {
				socket().sendBytes(buffer, n);
			}
		} catch (const std::exception& ex) {
			Poco::Logger::get("VpnServer").warning(Poco::format("Connection error: %s", ex.what()));
		}
	}
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

	SecureServerSocket svs(Poco::Net::SocketAddress(_config.address, _config.port), 64, _sslContext.get());
	auto params = new TCPServerParams;
	params->setMaxThreads(16);
	params->setMaxQueued(64);
	params->setThreadIdleTime(Poco::Timespan(10, 0));

	_tcpServer = std::make_unique<TCPServer>(new TCPServerConnectionFactoryImpl<Connection>(), svs, params);
	_tcpServer->start();
	_running = true;
	Poco::Logger::get("VpnServer").information("VPN server started");
}

void VpnServer::stop() {
	if (!_running) return;
	_tcpServer->stop();
	_tcpServer.reset();
	_running = false;
	Poco::Logger::get("VpnServer").information("VPN server stopped");
}

} // namespace vpn


