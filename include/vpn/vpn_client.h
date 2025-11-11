#pragma once

#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/PrivateKeyPassphraseHandler.h>
#include <Poco/Net/InvalidCertificateHandler.h>
#include <memory>
#include <string>
#include <vector>

namespace vpn {

struct ClientConfig {
	std::string serverHost = "127.0.0.1";
	unsigned short serverPort = 44350;
	std::string certFile = "certs/client.crt";
	std::string keyFile = "certs/client.key";
	std::string caFile = "certs/ca.crt";
	bool verifyServer = true;
};

class VpnClient {
public:
	explicit VpnClient(const ClientConfig& config);

	void connect();
	void disconnect();

	// Placeholder for sending encrypted payloads over the tunnel
	void send(const std::vector<unsigned char>& data);
	std::vector<unsigned char> receive();

private:
	ClientConfig _config;
	std::shared_ptr<Poco::Net::Context> _sslContext;
	std::unique_ptr<Poco::Net::SecureStreamSocket> _socket;
	bool _connected = false;
};

} // namespace vpn


