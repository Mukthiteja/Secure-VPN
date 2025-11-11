#pragma once

#include "vpn/core/Session.h"
#include <optional>
#include <string>

namespace vpn::core {

class AuthService {
public:
	// For Day 1 scaffold: accept any user with a non-empty token
	std::optional<Session> authenticate(const std::string& username, const std::string& bearerToken);
};

} // namespace vpn::core


