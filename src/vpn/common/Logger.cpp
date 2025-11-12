#include "vpn/common/Logger.h"

namespace vpn::common {

static bool g_initialized = false;

void LoggerFactory::initializeOnce() {
	if (g_initialized) return;
	auto console = new Poco::ConsoleChannel;
	Poco::AutoPtr<Poco::Formatter> formatter = new Poco::PatternFormatter("%L %Y-%m-%d %H:%M:%S.%i [%p] %s: %t");
	Poco::AutoPtr<Poco::Channel> channel = new Poco::FormattingChannel(formatter, console);
	Poco::Logger::root().setChannel(channel);
	Poco::Logger::root().setLevel("information");
	g_initialized = true;
}

Poco::Logger& LoggerFactory::get(const std::string& name) {
	initializeOnce();
	return Poco::Logger::get(name);
}

} // namespace vpn::common



