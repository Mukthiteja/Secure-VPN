#pragma once

#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/PrivateKeyPassphraseHandler.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/AutoPtr.h>
#include <memory>
#include <string>

namespace vpn {

struct ServerConfig {
	std::string address = "0.0.0.0";
	unsigned short port = 44350;
	std::string certFile = "certs/server.crt";
	std::string keyFile = "certs/server.key";
	std::string caFile = "certs/ca.crt";
	bool requireClientAuth = true;
};

class VpnServer {
public:
	explicit VpnServer(const ServerConfig& config);
	~VpnServer();

	void start();
	void stop();

private:
	class Connection;

	ServerConfig _config;
	std::unique_ptr<Poco::Net::TCPServer> _tcpServer;
	std::shared_ptr<Poco::Net::Context> _sslContext;
	bool _running = false;
};

} // namespace vpn


