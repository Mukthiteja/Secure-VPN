#include "vpn/core/Auth.h"
#include "vpn/core/Crypto.h"

namespace vpn::core {

std::optional<Session> AuthService::authenticate(const std::string& username, const std::string& bearerToken) {
	if (username.empty() || bearerToken.empty()) return std::nullopt;
	Session s{};
	s.id = Crypto::randomHex(16);
	s.user = username;
	s.keyMaterialHex = Crypto::randomHex(32);
	return s;
}

} // namespace vpn::core



