#include "vpn/crypto.h"
#include <Poco/Random.h>
#include <vector>
#include <cstring>

void test_crypto() {
	TEST_SUITE(Crypto) {
		// Test key derivation
		std::vector<std::uint8_t> keySeed(32, 0x42);
		std::vector<std::uint8_t> clientNonce(16, 0x11);
		std::vector<std::uint8_t> serverNonce(16, 0x22);
		
		auto keys = vpn::deriveSessionKeys(keySeed, clientNonce, serverNonce);
		ASSERT(keys.encKey.size() == 32, "Encryption key should be 32 bytes");
		ASSERT(keys.macKey.size() == 32, "MAC key should be 32 bytes");
		
		// Test encryption/decryption
		vpn::SessionCrypto crypto(keys.encKey, keys.macKey);
		std::vector<std::uint8_t> plaintext = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
		
		auto ciphertext = crypto.encrypt(plaintext);
		ASSERT(!ciphertext.empty(), "Ciphertext should not be empty");
		ASSERT(ciphertext.size() > plaintext.size(), "Ciphertext should be larger than plaintext (IV + padding)");
		
		auto decrypted = crypto.decrypt(ciphertext);
		ASSERT(decrypted.size() == plaintext.size(), "Decrypted size should match plaintext");
		ASSERT(memcmp(decrypted.data(), plaintext.data(), plaintext.size()) == 0, "Decrypted data should match plaintext");
		
		// Test that same plaintext produces different ciphertext (IV randomization)
		auto ciphertext2 = crypto.encrypt(plaintext);
		ASSERT(ciphertext != ciphertext2, "Same plaintext should produce different ciphertext due to IV");
		
		// Test MAC verification failure
		ciphertext[0] ^= 0xFF; // Corrupt ciphertext
		bool caught = false;
		try {
			crypto.decrypt(ciphertext);
		} catch (const std::exception&) {
			caught = true;
		}
		ASSERT(caught, "Corrupted ciphertext should fail MAC verification");
	}
}

