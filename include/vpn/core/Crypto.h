#pragma once

#include <string>
#include <vector>

namespace vpn::core {

class Crypto {
public:
	static std::vector<unsigned char> sha256(const std::vector<unsigned char>& data);
	static std::string randomHex(std::size_t bytes);
};

} // namespace vpn::core


