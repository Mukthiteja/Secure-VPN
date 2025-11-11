#pragma once

#include "vpn/core/Session.h"
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <string>

namespace vpn::tunnel {

// Day 1: placeholder for future secure data channel (TCP for now)
class Tunnel {
public:
	explicit Tunnel(const vpn::core::Session& session);
	bool isOpen() const;
private:
	vpn::core::Session session_;
	bool open_ { true };
};

} // namespace vpn::tunnel


