#include "vpn/tunnel.h"

#include <Poco/Timespan.h>
#include <stdexcept>
#include <algorithm>

namespace vpn {

Tunnel::Tunnel(Poco::Net::SecureStreamSocket& socket)
	: _socket(socket) {}

void Tunnel::clientHandshake(const std::string& clientSessionId) {
	Frame hello{FrameType::HELLO, std::vector<std::uint8_t>(clientSessionId.begin(), clientSessionId.end())};
	sendFrame(hello);
	Frame ack;
	if (!receiveFrame(ack, std::chrono::milliseconds(5000)) || ack.type != FrameType::HELLO_ACK) {
		throw std::runtime_error("HELLO_ACK not received");
	}
}

std::string Tunnel::serverHandshake(const std::string& serverSessionId) {
	Frame hello;
	if (!receiveFrame(hello, std::chrono::milliseconds(5000)) || hello.type != FrameType::HELLO) {
		throw std::runtime_error("HELLO not received");
	}
	Frame ack{FrameType::HELLO_ACK, std::vector<std::uint8_t>(serverSessionId.begin(), serverSessionId.end())};
	sendFrame(ack);
	return std::string(hello.payload.begin(), hello.payload.end());
}

void Tunnel::sendData(const std::vector<std::uint8_t>& data) {
	Frame f{FrameType::DATA, data};
	sendFrame(f);
}

std::vector<std::uint8_t> Tunnel::receiveData(std::chrono::milliseconds timeout) {
	Frame f;
	if (!receiveFrame(f, timeout)) return {};
	if (f.type == FrameType::DATA) return f.payload;
	return {};
}

void Tunnel::sendHeartbeat() {
	Frame f{FrameType::HEARTBEAT, {}};
	sendFrame(f);
}

bool Tunnel::receiveHeartbeat(std::chrono::milliseconds timeout) {
	Frame f;
	if (!receiveFrame(f, timeout)) return false;
	return f.type == FrameType::HEARTBEAT;
}

void Tunnel::sendClose() {
	Frame f{FrameType::CLOSE, {}};
	sendFrame(f);
}

void Tunnel::sendFrame(const Frame& frame) {
	// Frame format: [len:4][type:1][payload...], len = 1 + payload size
	std::vector<std::uint8_t> buf;
	buf.reserve(5 + frame.payload.size());
	writeUint32(buf, static_cast<std::uint32_t>(1 + frame.payload.size()));
	buf.push_back(static_cast<std::uint8_t>(frame.type));
	buf.insert(buf.end(), frame.payload.begin(), frame.payload.end());
	const char* data = reinterpret_cast<const char*>(buf.data());
	int toSend = static_cast<int>(buf.size());
	int sent = 0;
	while (sent < toSend) {
		int n = _socket.sendBytes(data + sent, toSend - sent);
		if (n <= 0) throw std::runtime_error("sendFrame failed");
		sent += n;
	}
}

bool Tunnel::receiveFrame(Frame& outFrame, std::chrono::milliseconds timeout) {
	_socket.setReceiveTimeout(Poco::Timespan(0, static_cast<long>(timeout.count()) * 1000));
	std::uint8_t hdr[4];
	int recvd = 0;
	while (recvd < 4) {
		int n = _socket.receiveBytes(reinterpret_cast<void*>(hdr + recvd), 4 - recvd);
		if (n <= 0) return false;
		recvd += n;
	}
	std::uint32_t len = readUint32(hdr);
	if (len == 0) return false;
	std::vector<std::uint8_t> body(len);
	int got = 0;
	while (got < static_cast<int>(len)) {
		int n = _socket.receiveBytes(reinterpret_cast<void*>(body.data() + got), static_cast<int>(len) - got);
		if (n <= 0) return false;
		got += n;
	}
	if (body.empty()) return false;
	outFrame.type = static_cast<FrameType>(body[0]);
	outFrame.payload.assign(body.begin() + 1, body.end());
	return true;
}

void Tunnel::writeUint32(std::vector<std::uint8_t>& buf, std::uint32_t v) {
	// network byte order (big endian)
	buf.push_back(static_cast<std::uint8_t>((v >> 24) & 0xFF));
	buf.push_back(static_cast<std::uint8_t>((v >> 16) & 0xFF));
	buf.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
	buf.push_back(static_cast<std::uint8_t>(v & 0xFF));
}

std::uint32_t Tunnel::readUint32(const std::uint8_t* p) {
	return (static_cast<std::uint32_t>(p[0]) << 24) |
	       (static_cast<std::uint32_t>(p[1]) << 16) |
	       (static_cast<std::uint32_t>(p[2]) << 8) |
	       (static_cast<std::uint32_t>(p[3]));
}

} // namespace vpn


