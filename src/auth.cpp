#include "vpn/auth.h"

#include <Poco/File.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/StreamCopier.h>
#include <Poco/Base64Decoder.h>
#include <Poco/SHA2Engine.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace vpn {

static std::vector<std::uint8_t> base64Decode(const std::string& input) {
	std::istringstream istr(input);
	Poco::Base64Decoder decoder(istr);
	std::vector<std::uint8_t> output;
	char ch;
	while (decoder.get(ch)) {
		output.push_back(static_cast<std::uint8_t>(ch));
	}
	return output;
}

std::vector<std::uint8_t> computePasswordHash(const std::vector<std::uint8_t>& salt,
                                              const std::string& password) {
	Poco::SHA2Engine sha256;
	sha256.update(salt.data(), static_cast<unsigned>(salt.size()));
	sha256.update(password);
	const auto& digest = sha256.digest();
	return std::vector<std::uint8_t>(digest.begin(), digest.end());
}

CredentialStore::Ptr CredentialStore::loadFromFile(const std::string& path) {
	Poco::File file(path);
	if (!file.exists()) {
		throw std::runtime_error("Credential file not found: " + path);
	}
	std::ifstream ifs(path, std::ios::binary);
	if (!ifs) throw std::runtime_error("Failed to open credential file: " + path);
	std::stringstream buffer;
	buffer << ifs.rdbuf();
	Poco::JSON::Parser parser;
	auto result = parser.parse(buffer.str());
	auto obj = result.extract<Poco::JSON::Object::Ptr>();
	auto users = obj->getArray("users");
	if (!users) throw std::runtime_error("Credential file missing 'users' array");
	auto store = std::make_shared<CredentialStore>();
	for (size_t i = 0; i < users->size(); ++i) {
		auto userObj = users->getObject(i);
		UserRecord rec;
		rec.username = userObj->getValue<std::string>("username");
		if (userObj->has("salt")) {
			rec.salt = base64Decode(userObj->getValue<std::string>("salt"));
		}
		if (userObj->has("hash")) {
			rec.hash = base64Decode(userObj->getValue<std::string>("hash"));
		}
		if (userObj->has("password")) {
			rec.passwordPlain = userObj->getValue<std::string>("password");
		}
		store->_records.emplace(rec.username, std::move(rec));
	}
	return store;
}

bool CredentialStore::verify(const std::string& username, const std::string& password) const {
	auto it = _records.find(username);
	if (it == _records.end()) return false;
	if (!it->second.hash.empty() && !it->second.salt.empty()) {
		auto computed = computePasswordHash(it->second.salt, password);
		return computed == it->second.hash;
	}
	if (!it->second.passwordPlain.empty()) {
		return password == it->second.passwordPlain;
	}
	return false;
}

} // namespace vpn


