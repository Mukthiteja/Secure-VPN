#include <Poco/Net/SecureServerSocket.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/SharedPtr.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/HelpFormatter.h>
#include "vpn/common/Logger.h"

using Poco::Net::Context;
using Poco::Net::SSLManager;
using Poco::Net::PrivateKeyPassphraseHandler;
using Poco::Net::InvalidCertificateHandler;
using Poco::Net::SecureServerSocket;
using Poco::Net::TCPServer;
using Poco::Net::TCPServerConnection;
using Poco::Net::TCPServerConnectionFactory;

class DummyPassphraseHandler : public PrivateKeyPassphraseHandler {
public:
	explicit DummyPassphraseHandler(bool) : PrivateKeyPassphraseHandler(true) {}
	void onPrivateKeyRequested(const void*, std::string& passphrase) override { passphrase.clear(); }
};

class ServerConn : public TCPServerConnection {
public:
	using TCPServerConnection::TCPServerConnection;
	void run() override {
		Poco::Logger& log = vpn::common::LoggerFactory::get("server.conn");
		log.information("Accepted connection");
		try {
			std::string hello = "CustomVPN server ready\n";
			socket().sendBytes(hello.data(), (int)hello.size());
		} catch (const std::exception& e) {
			log.error(std::string("Connection error: ") + e.what());
		}
	}
};

class VpnServerApp : public Poco::Util::ServerApplication {
public:
	VpnServerApp() = default;
	~VpnServerApp() override = default;
protected:
	void initialize(Application& self) override {
		loadConfiguration();
		ServerApplication::initialize(self);
	}

	void uninitialize() override {
		ServerApplication::uninitialize();
	}

	int main(const std::vector<std::string>&) override {
		Poco::Logger& log = vpn::common::LoggerFactory::get("server");
		Poco::SharedPtr<PrivateKeyPassphraseHandler> pPass = new DummyPassphraseHandler(false);
		Poco::SharedPtr<InvalidCertificateHandler> pCert = new Poco::Net::AcceptCertificateHandler(false);
		Context::Ptr pContext = new Context(Context::SERVER_USE, "", "", "",
			Context::VERIFY_NONE, 9, true, "ALL");
		SSLManager::instance().initializeServer(pPass, pCert, pContext);

		SecureServerSocket sss(8443);
		TCPServer server(new TCPServerConnectionFactoryImpl<ServerConn>(), sss);
		server.start();
		log.information("Server listening on 0.0.0.0:8443 (TLS)");
		waitForTerminationRequest();
		server.stop();
		return Application::EXIT_OK;
	}
};

POCO_SERVER_MAIN(VpnServerApp)


