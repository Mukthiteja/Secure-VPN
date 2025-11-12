#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace vpn {

struct UserRecord {
	std::string username;
	std::vector<std::uint8_t> salt;
	std::vector<std::uint8_t> hash;
	std::string passwordPlain;
};

class CredentialStore {
public:
	using Ptr = std::shared_ptr<CredentialStore>;

	static Ptr loadFromFile(const std::string& path);

	bool verify(const std::string& username, const std::string& password) const;

private:
	std::unordered_map<std::string, UserRecord> _records;
};

std::vector<std::uint8_t> computePasswordHash(const std::vector<std::uint8_t>& salt,
                                              const std::string& password);

} // namespace vpn


