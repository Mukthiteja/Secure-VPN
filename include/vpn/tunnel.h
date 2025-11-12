#pragma once

#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Types.h>
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>

namespace vpn {

enum class FrameType : std::uint8_t {
	HELLO = 1,
	HELLO_ACK = 2,
	DATA = 3,
	HEARTBEAT = 4,
	CLOSE = 5,
	ENCRYPTED_DATA = 6
};

struct Frame {
	FrameType type;
	std::vector<std::uint8_t> payload;
};

class Tunnel {
public:
	explicit Tunnel(Poco::Net::SecureStreamSocket& socket);

	// Handshake (extended):
	// Client sends HELLO: [idLen:1][id][clientNonce:16]
	// Server replies HELLO_ACK: [idLen:1][id][serverNonce:16][keySeed:32]
	void clientHandshake(const std::string& clientSessionId,
	                     std::vector<std::uint8_t>& outClientNonce,
	                     std::string& outServerSessionId,
	                     std::vector<std::uint8_t>& outServerNonce,
	                     std::vector<std::uint8_t>& outKeySeed);
	void serverHandshake(const std::string& serverSessionId,
	                     std::string& outClientSessionId,
	                     std::vector<std::uint8_t>& outClientNonce,
	                     std::vector<std::uint8_t>& outServerNonce,
	                     std::vector<std::uint8_t>& outKeySeed);

	// Data
	void sendData(const std::vector<std::uint8_t>& data);
	std::vector<std::uint8_t> receiveData(std::chrono::milliseconds timeout);
	void sendEncrypted(const std::vector<std::uint8_t>& cipherFrame);
	std::vector<std::uint8_t> receiveEncrypted(std::chrono::milliseconds timeout);

	// Heartbeat
	void sendHeartbeat();
	bool receiveHeartbeat(std::chrono::milliseconds timeout);

	// Close
	void sendClose();

private:
	void sendFrame(const Frame& frame);
	bool receiveFrame(Frame& outFrame, std::chrono::milliseconds timeout);
	static void writeUint32(std::vector<std::uint8_t>& buf, std::uint32_t v);
	static std::uint32_t readUint32(const std::uint8_t* p);

	Poco::Net::SecureStreamSocket& _socket;
};

} // namespace vpn


