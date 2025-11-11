#pragma once

#include <string>
#include <memory>

namespace vpn::core {

struct Session {
	std::string id;
	std::string user;
	std::string keyMaterialHex;
};

} // namespace vpn::core


