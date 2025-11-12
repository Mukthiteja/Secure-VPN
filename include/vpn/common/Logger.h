#pragma once

#include <Poco/Logger.h>
#include <Poco/AutoPtr.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include <Poco/ConsoleChannel.h>
#include <memory>

namespace vpn::common {

class LoggerFactory {
public:
	static Poco::Logger& get(const std::string& name);
	static void initializeOnce();
};

} // namespace vpn::common



