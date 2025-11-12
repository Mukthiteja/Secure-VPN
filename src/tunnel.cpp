#include "vpn/tunnel.h"

#include <Poco/Timespan.h>
#include <stdexcept>
#include <algorithm>

namespace vpn {

Tunnel::Tunnel(Poco::Net::SecureStreamSocket& socket)
	: _socket(socket) {}

void Tunnel::clientHandshake(const std::string& clientSessionId,
                             std::vector<std::uint8_t>& outClientNonce,
                             std::string& outServerSessionId,
                             std::vector<std::uint8_t>& outServerNonce,
                             std::vector<std::uint8_t>& outKeySeed) {
	// build HELLO: [idLen][id][clientNonce(16)]
	std::vector<std::uint8_t> payload;
	if (clientSessionId.size() > 255) throw std::runtime_error("client id too long");
	payload.push_back(static_cast<std::uint8_t>(clientSessionId.size()));
	payload.insert(payload.end(), clientSessionId.begin(), clientSessionId.end());
	std::vector<std::uint8_t> clientNonce(16);
	Poco::RandomBuf rng;
	rng.read(reinterpret_cast<char*>(clientNonce.data()), 16);
	outClientNonce = clientNonce;
	payload.insert(payload.end(), clientNonce.begin(), clientNonce.end());
	Frame hello{FrameType::HELLO, payload};
	sendFrame(hello);
	Frame ack;
	if (!receiveFrame(ack, std::chrono::milliseconds(5000)) || ack.type != FrameType::HELLO_ACK) {
		throw std::runtime_error("HELLO_ACK not received");
	}
	// parse ACK: [idLen][id][serverNonce(16)][keySeed(32)]
	if (ack.payload.size() < 1 + 16 + 32) throw std::runtime_error("HELLO_ACK payload too short");
	std::size_t p = 0;
	std::size_t idLen = ack.payload[p++];
	if (ack.payload.size() < 1 + idLen + 16 + 32) throw std::runtime_error("HELLO_ACK payload too short");
	outServerSessionId.assign(reinterpret_cast<const char*>(ack.payload.data() + p), idLen);
	p += idLen;
	outServerNonce.assign(ack.payload.begin() + p, ack.payload.begin() + p + 16);
	p += 16;
	outKeySeed.assign(ack.payload.begin() + p, ack.payload.begin() + p + 32);
}

void Tunnel::serverHandshake(const std::string& serverSessionId,
                             std::string& outClientSessionId,
                             std::vector<std::uint8_t>& outClientNonce,
                             std::vector<std::uint8_t>& outServerNonce,
                             std::vector<std::uint8_t>& outKeySeed) {
	Frame hello;
	if (!receiveFrame(hello, std::chrono::milliseconds(5000)) || hello.type != FrameType::HELLO) {
		throw std::runtime_error("HELLO not received");
	}
	// parse HELLO: [idLen][id][clientNonce(16)]
	if (hello.payload.size() < 1 + 16) throw std::runtime_error("HELLO payload too short");
	std::size_t p = 0;
	std::size_t idLen = hello.payload[p++];
	if (hello.payload.size() < 1 + idLen + 16) throw std::runtime_error("HELLO payload too short");
	outClientSessionId.assign(reinterpret_cast<const char*>(hello.payload.data() + p), idLen);
	p += idLen;
	outClientNonce.assign(hello.payload.begin() + p, hello.payload.begin() + p + 16);
	// build ACK with serverNonce and keySeed
	outServerNonce.assign(16, 0);
	Poco::RandomBuf rng;
	rng.read(reinterpret_cast<char*>(outServerNonce.data()), 16);
	outKeySeed.assign(32, 0);
	rng.read(reinterpret_cast<char*>(outKeySeed.data()), 32);
	std::vector<std::uint8_t> payload;
	if (serverSessionId.size() > 255) throw std::runtime_error("server id too long");
	payload.push_back(static_cast<std::uint8_t>(serverSessionId.size()));
	payload.insert(payload.end(), serverSessionId.begin(), serverSessionId.end());
	payload.insert(payload.end(), outServerNonce.begin(), outServerNonce.end());
	payload.insert(payload.end(), outKeySeed.begin(), outKeySeed.end());
	Frame ack{FrameType::HELLO_ACK, payload};
	sendFrame(ack);
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

void Tunnel::sendEncrypted(const std::vector<std::uint8_t>& cipherFrame) {
	Frame f{FrameType::ENCRYPTED_DATA, cipherFrame};
	sendFrame(f);
}

std::vector<std::uint8_t> Tunnel::receiveEncrypted(std::chrono::milliseconds timeout) {
	Frame f;
	if (!receiveFrame(f, timeout)) return {};
	if (f.type == FrameType::ENCRYPTED_DATA) return f.payload;
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


