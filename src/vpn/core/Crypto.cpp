#include "vpn/core/Crypto.h"
#include <Poco/Crypto/DigestEngine.h>
#include <Poco/RandomStream.h>
#include <Poco/HexBinaryEncoder.h>
#include <sstream>

namespace vpn::core {

std::vector<unsigned char> Crypto::sha256(const std::vector<unsigned char>& data) {
	Poco::Crypto::DigestEngine engine("SHA256");
	engine.update(data);
	const auto& digest = engine.digest();
	return std::vector<unsigned char>(digest.begin(), digest.end());
}

std::string Crypto::randomHex(std::size_t bytes) {
	std::string buffer(bytes, '\0');
	Poco::RandomInputStream ris;
	ris.read(buffer.data(), static_cast<std::streamsize>(bytes));
	std::ostringstream oss;
	Poco::HexBinaryEncoder encoder(oss);
	encoder.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
	encoder.close();
	return oss.str();
}

} // namespace vpn::core


