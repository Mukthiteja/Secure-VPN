#include "vpn/vpn_server.h"
#include "vpn/vpn_client.h"

#include <Poco/Util/Application.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Logger.h>
#include <iostream>
#include <memory>

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;

class CustomVpnApp : public Application {
public:
	CustomVpnApp() : _helpRequested(false) {}

protected:
	void initialize(Application& self) override {
		Application::initialize(self);
	}

	void uninitialize() override {
		Application::uninitialize();
	}

	void defineOptions(OptionSet& options) override {
		Application::defineOptions(options);
		options.addOption(
			Option("help", "h", "Display help information").required(false).repeatable(false));
		options.addOption(
			Option("mode", "m", "Mode: server|client")
				.argument("mode")
				.required(false)
				.repeatable(false));
		options.addOption(
			Option("credentials", "c", "Path to credential store JSON (server only)")
				.argument("file")
				.required(false)
				.repeatable(false));
		options.addOption(
			Option("username", "u", "Username for client authentication")
				.argument("username")
				.required(false)
				.repeatable(false));
		options.addOption(
			Option("password", "p", "Password for client authentication")
				.argument("password")
				.required(false)
				.repeatable(false));
	}

	void handleOption(const std::string& name, const std::string& value) override {
		Application::handleOption(name, value);
		if (name == "help") {
			_helpRequested = true;
		} else if (name == "mode") {
			_mode = value;
		} else if (name == "credentials") {
			_credentialFile = value;
		} else if (name == "username") {
			_username = value;
		} else if (name == "password") {
			_password = value;
		}
	}

	int main(const std::vector<std::string>&) override {
		if (_helpRequested) {
			HelpFormatter helpFormatter(options());
			helpFormatter.setCommand(commandName());
			helpFormatter.setUsage("[-m server|client] [--credentials file] [--username user --password pass]");
			helpFormatter.setHeader("Custom VPN application powered by Poco.");
			helpFormatter.format(std::cout);
			return Application::EXIT_OK;
		}

		if (_mode == "server") {
			vpn::ServerConfig cfg;
			if (!_credentialFile.empty()) cfg.credentialFile = _credentialFile;
			vpn::VpnServer server(cfg);
			server.start();
			waitForTerminationRequest();
			server.stop();
		} else if (_mode == "client") {
			vpn::ClientConfig cfg;
			if (!_username.empty()) cfg.username = _username;
			if (!_password.empty()) cfg.password = _password;
			vpn::VpnClient client(cfg);
			client.connect();
			// Demo: send framed data and print size of echo
			auto payload = std::vector<unsigned char>{'P','I','N','G'};
			client.send(payload);
			auto resp = client.receive();
			if (!resp.empty()) {
				std::cout << "Received " << resp.size() << " bytes\n";
			}
			client.disconnect();
		} else {
			logger().information("No mode specified. Use --mode=server or --mode=client.");
		}
		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
	std::string _mode;
	std::string _credentialFile;
	std::string _username;
	std::string _password;
};

POCO_APP_MAIN(CustomVpnApp)


