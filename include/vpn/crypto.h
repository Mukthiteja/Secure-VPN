#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace vpn {

struct DerivedKeys {
	std::vector<std::uint8_t> encKey; // 32 bytes
	std::vector<std::uint8_t> macKey; // 32 bytes
};

// HKDF-SHA256: derive outLen bytes from ikm with salt and info
std::vector<std::uint8_t> hkdfSha256(const std::vector<std::uint8_t>& ikm,
                                     const std::vector<std::uint8_t>& salt,
                                     const std::vector<std::uint8_t>& info,
                                     std::size_t outLen);

// Derive 64 bytes, split into encKey(32) and macKey(32)
DerivedKeys deriveSessionKeys(const std::vector<std::uint8_t>& keySeed,
                              const std::vector<std::uint8_t>& clientNonce,
                              const std::vector<std::uint8_t>& serverNonce);

class SessionCrypto {
public:
	SessionCrypto(const std::vector<std::uint8_t>& encKey,
	              const std::vector<std::uint8_t>& macKey);

	// Encrypt-then-MAC
	// ciphertext frame format: [ivLen:1][iv][ciphertext][hmac(32)]
	std::vector<std::uint8_t> encrypt(const std::vector<std::uint8_t>& plaintext) const;
	std::vector<std::uint8_t> decrypt(const std::vector<std::uint8_t>& frame) const;

private:
	std::vector<std::uint8_t> _encKey;
	std::vector<std::uint8_t> _macKey;
};

} // namespace vpn


