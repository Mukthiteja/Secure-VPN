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
	}

	void handleOption(const std::string& name, const std::string& value) override {
		Application::handleOption(name, value);
		if (name == "help") {
			_helpRequested = true;
		} else if (name == "mode") {
			_mode = value;
		}
	}

	int main(const std::vector<std::string>&) override {
		if (_helpRequested) {
			HelpFormatter helpFormatter(options());
			helpFormatter.setCommand(commandName());
			helpFormatter.setUsage("[-m server|client]");
			helpFormatter.setHeader("Custom VPN application powered by Poco.");
			helpFormatter.format(std::cout);
			return Application::EXIT_OK;
		}

		if (_mode == "server") {
			vpn::ServerConfig cfg;
			vpn::VpnServer server(cfg);
			server.start();
			waitForTerminationRequest();
			server.stop();
		} else if (_mode == "client") {
			vpn::ClientConfig cfg;
			vpn::VpnClient client(cfg);
			client.connect();
			// Minimal demo interaction; in future we will integrate real tunnel I/O
			auto hello = std::vector<unsigned char>{'P','I','N','G','\n'};
			client.send(hello);
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
};

POCO_APP_MAIN(CustomVpnApp)


