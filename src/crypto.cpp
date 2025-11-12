#include "vpn/crypto.h"

#include <Poco/Crypto/Cipher.h>
#include <Poco/Crypto/CipherKey.h>
#include <Poco/Crypto/CryptoTransform.h>
#include <Poco/Crypto/RSAKey.h>
#include <Poco/HMACEngine.h>
#include <Poco/SHA2Engine.h>
#include <Poco/RandomBuf.h>
#include <stdexcept>
#include <cstring>

namespace vpn {

static std::vector<std::uint8_t> randomBytes(std::size_t len) {
	std::vector<std::uint8_t> v(len);
	Poco::RandomBuf rng;
	rng.read(reinterpret_cast<char*>(v.data()), static_cast<std::streamsize>(len));
	return v;
}

std::vector<std::uint8_t> hkdfSha256(const std::vector<std::uint8_t>& ikm,
                                     const std::vector<std::uint8_t>& salt,
                                     const std::vector<std::uint8_t>& info,
                                     std::size_t outLen) {
	// HKDF-Extract
	Poco::HMACEngine<Poco::SHA2Engine> hmacExtract(std::string(reinterpret_cast<const char*>(salt.data()), salt.size()));
	hmacExtract.update(ikm.data(), static_cast<unsigned>(ikm.size()));
	const auto prkStr = hmacExtract.digest(); // 32 bytes
	std::vector<std::uint8_t> prk(prkStr.begin(), prkStr.end());

	// HKDF-Expand
	std::vector<std::uint8_t> okm;
	okm.reserve(outLen);
	std::vector<std::uint8_t> t;
	unsigned char counter = 1;
	while (okm.size() < outLen) {
		Poco::HMACEngine<Poco::SHA2Engine> hmacExpand(std::string(reinterpret_cast<const char*>(prk.data()), prk.size()));
		if (!t.empty()) hmacExpand.update(t.data(), static_cast<unsigned>(t.size()));
		if (!info.empty()) hmacExpand.update(info.data(), static_cast<unsigned>(info.size()));
		hmacExpand.update(&counter, 1);
		const auto block = hmacExpand.digest();
		t.assign(block.begin(), block.end());
		std::size_t toCopy = std::min<std::size_t>(t.size(), outLen - okm.size());
		okm.insert(okm.end(), t.begin(), t.begin() + toCopy);
		counter++;
	}
	return okm;
}

DerivedKeys deriveSessionKeys(const std::vector<std::uint8_t>& keySeed,
                              const std::vector<std::uint8_t>& clientNonce,
                              const std::vector<std::uint8_t>& serverNonce) {
	std::vector<std::uint8_t> salt;
	salt.reserve(clientNonce.size() + serverNonce.size());
	salt.insert(salt.end(), clientNonce.begin(), clientNonce.end());
	salt.insert(salt.end(), serverNonce.begin(), serverNonce.end());
	static const std::vector<std::uint8_t> info = {'C','u','s','t','o','m','V','p','n','-','v','1'};
	auto okm = hkdfSha256(keySeed, salt, info, 64);
	DerivedKeys out;
	out.encKey.assign(okm.begin(), okm.begin() + 32);
	out.macKey.assign(okm.begin() + 32, okm.begin() + 64);
	return out;
}

SessionCrypto::SessionCrypto(const std::vector<std::uint8_t>& encKey,
                             const std::vector<std::uint8_t>& macKey)
	: _encKey(encKey), _macKey(macKey) {
	if (_encKey.size() != 32 || _macKey.size() != 32) {
		throw std::invalid_argument("SessionCrypto requires 32-byte encKey and macKey");
	}
}

std::vector<std::uint8_t> SessionCrypto::encrypt(const std::vector<std::uint8_t>& plaintext) const {
	// AES-256-CBC with random 16-byte IV, then HMAC-SHA256 over (ivLen|iv|ciphertext)
	const std::size_t ivLen = 16;
	auto iv = randomBytes(ivLen);
	Poco::Crypto::CipherKey key("aes-256-cbc",
		std::string(reinterpret_cast<const char*>(_encKey.data()), _encKey.size()),
		std::string(reinterpret_cast<const char*>(iv.data()), iv.size()));
	auto cipher = Poco::Crypto::Cipher::createCipher(key);
	std::string encrypted = cipher->encrypt(std::string(reinterpret_cast<const char*>(plaintext.data()), plaintext.size()));

	std::vector<std::uint8_t> frame;
	frame.reserve(1 + iv.size() + encrypted.size() + 32);
	frame.push_back(static_cast<std::uint8_t>(ivLen));
	frame.insert(frame.end(), iv.begin(), iv.end());
	frame.insert(frame.end(), encrypted.begin(), encrypted.end());

	Poco::HMACEngine<Poco::SHA2Engine> hmac(std::string(reinterpret_cast<const char*>(_macKey.data()), _macKey.size()));
	hmac.update(frame.data(), static_cast<unsigned>(frame.size()));
	const auto macStr = hmac.digest();
	frame.insert(frame.end(), macStr.begin(), macStr.end());
	return frame;
}

std::vector<std::uint8_t> SessionCrypto::decrypt(const std::vector<std::uint8_t>& frame) const {
	if (frame.size() < 1 + 16 + 32) throw std::runtime_error("cipher frame too short");
	std::size_t ivLen = frame[0];
	if (ivLen != 16) throw std::runtime_error("invalid iv length");
	if (frame.size() < 1 + ivLen + 32) throw std::runtime_error("cipher frame too short");
	const std::size_t macOffset = frame.size() - 32;
	// verify HMAC
	Poco::HMACEngine<Poco::SHA2Engine> hmac(std::string(reinterpret_cast<const char*>(_macKey.data()), _macKey.size()));
	hmac.update(frame.data(), static_cast<unsigned>(macOffset));
	const auto macStr = hmac.digest();
	std::vector<std::uint8_t> expected(macStr.begin(), macStr.end());
	if (!std::equal(expected.begin(), expected.end(), frame.begin() + macOffset)) {
		throw std::runtime_error("HMAC verification failed");
	}
	// decrypt
	std::vector<std::uint8_t> iv(frame.begin() + 1, frame.begin() + 1 + ivLen);
	std::vector<std::uint8_t> ciphertext(frame.begin() + 1 + ivLen, frame.begin() + macOffset);
	Poco::Crypto::CipherKey key("aes-256-cbc",
		std::string(reinterpret_cast<const char*>(_encKey.data()), _encKey.size()),
		std::string(reinterpret_cast<const char*>(iv.data()), iv.size()));
	auto cipher = Poco::Crypto::Cipher::createCipher(key);
	std::string decrypted = cipher->decrypt(std::string(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size()));
	return std::vector<std::uint8_t>(decrypted.begin(), decrypted.end());
}

} // namespace vpn


