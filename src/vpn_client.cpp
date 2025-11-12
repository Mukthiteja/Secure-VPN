#include "vpn/vpn_client.h"

#include <Poco/Net/SSLManager.h>
#include <Poco/Net/ConsoleCertificateHandler.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <stdexcept>
#include <Poco/UUIDGenerator.h>
#include "vpn/tunnel.h"
#include "vpn/crypto.h"

using Poco::Net::Context;
using Poco::Net::SecureStreamSocket;
using Poco::Net::SSLManager;

namespace vpn {

VpnClient::VpnClient(const ClientConfig& config)
	: _config(config) {
	_sslContext = std::make_shared<Context>(
		Context::CLIENT_USE,
		_config.keyFile,
		_config.certFile,
		_config.caFile,
		_config.verifyServer ? Context::VERIFY_STRICT : Context::VERIFY_NONE,
		9,
		true,
		"ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"
	);
	Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> certHandler = new Poco::Net::ConsoleCertificateHandler(true);
	Poco::SharedPtr<Poco::Net::PrivateKeyPassphraseHandler> pkeyHandler = new Poco::Net::KeyConsoleHandler(false);
	SSLManager::instance().initializeClient(pkeyHandler, certHandler, _sslContext.get());
}

void VpnClient::connect() {
	if (_connected) return;
	Poco::Net::SocketAddress addr(_config.serverHost, _config.serverPort);
	_socket = std::make_unique<SecureStreamSocket>(addr, _sslContext.get());
	// Perform tunnel handshake
	vpn::Tunnel tunnel(*_socket);
	auto clientSessionId = Poco::UUIDGenerator::defaultGenerator().createRandom().toString();
	std::string serverSessionId;
	std::vector<std::uint8_t> clientNonce, serverNonce, keySeed;
	tunnel.clientHandshake(clientSessionId, clientNonce, serverSessionId, serverNonce, keySeed);
	auto keys = vpn::deriveSessionKeys(keySeed, clientNonce, serverNonce);
	_sessionCrypto = std::make_unique<vpn::SessionCrypto>(keys.encKey, keys.macKey);
	_connected = true;
	Poco::Logger::get("VpnClient").information("Connected to VPN server");
}

void VpnClient::disconnect() {
	if (!_connected) return;
	try {
		_socket->shutdown();
	} catch (...) {}
	_socket.reset();
	_connected = false;
	Poco::Logger::get("VpnClient").information("Disconnected from VPN server");
}

void VpnClient::send(const std::vector<unsigned char>& data) {
	if (!_connected || !_socket) throw std::runtime_error("Not connected");
	vpn::Tunnel tunnel(*_socket);
	if (_sessionCrypto) {
		auto enc = _sessionCrypto->encrypt(data);
		tunnel.sendEncrypted(enc);
	} else {
		tunnel.sendData(data);
	}
}

std::vector<unsigned char> VpnClient::receive() {
	if (!_connected || !_socket) throw std::runtime_error("Not connected");
	vpn::Tunnel tunnel(*_socket);
	if (_sessionCrypto) {
		auto enc = tunnel.receiveEncrypted(std::chrono::milliseconds(5000));
		if (!enc.empty()) {
			return _sessionCrypto->decrypt(enc);
		}
	}
	return tunnel.receiveData(std::chrono::milliseconds(5000));
}

} // namespace vpn


