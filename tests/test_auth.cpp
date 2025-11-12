#include "vpn/auth.h"
#include <Poco/File.h>
#include <Poco/Path.h>
#include <fstream>
#include <sstream>

void test_auth() {
	TEST_SUITE(Auth) {
		// Create temporary credential file
		std::string testFile = "test_users.json";
		{
			std::ofstream ofs(testFile);
			ofs << R"({
	"users": [
		{
			"username": "testuser",
			"password": "testpass123"
		},
		{
			"username": "hasheduser",
			"salt": "dGVzdHNhbHQ=",
			"hash": "YWJjZGVmZ2hpams="
		}
	]
})";
		}
		
		// Test loading credential store
		auto store = vpn::CredentialStore::loadFromFile(testFile);
		ASSERT(store != nullptr, "Should load credential store");
		
		// Test plaintext password verification
		ASSERT(store->verify("testuser", "testpass123"), "Should verify correct plaintext password");
		ASSERT(!store->verify("testuser", "wrongpass"), "Should reject wrong password");
		ASSERT(!store->verify("nonexistent", "anypass"), "Should reject nonexistent user");
		
		// Cleanup
		Poco::File(testFile).remove();
	}
}

