#include "vpn/tunnel.h"
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Thread.h>
#include <vector>
#include <memory>

// Mock socket pair for testing
class MockSocketPair {
public:
	Poco::Net::StreamSocket clientSock;
	Poco::Net::StreamSocket serverSock;
	
	MockSocketPair() {
		Poco::Net::ServerSocket server(0);
		Poco::Net::SocketAddress addr("127.0.0.1", server.address().port());
		clientSock.connect(addr);
		serverSock = server.acceptConnection();
	}
};

void test_tunnel() {
	TEST_SUITE(Tunnel) {
		// Test frame send/receive
		MockSocketPair pair;
		vpn::Tunnel clientTunnel(pair.clientSock);
		vpn::Tunnel serverTunnel(pair.serverSock);
		
		// Test DATA frame
		std::vector<std::uint8_t> testData = {'T', 'e', 's', 't'};
		clientTunnel.sendData(testData);
		auto received = serverTunnel.receiveData(std::chrono::milliseconds(1000));
		ASSERT(!received.empty(), "Should receive data");
		ASSERT(received == testData, "Received data should match sent data");
		
		// Test HEARTBEAT
		clientTunnel.sendHeartbeat();
		bool gotHeartbeat = serverTunnel.receiveHeartbeat(std::chrono::milliseconds(1000));
		ASSERT(gotHeartbeat, "Should receive heartbeat");
		
		// Test handshake
		std::string clientId = "client-123";
		std::string serverId;
		std::vector<std::uint8_t> clientNonce, serverNonce, keySeed;
		
		Poco::Thread clientThread;
		clientThread.startFunc([&]() {
			clientTunnel.clientHandshake(clientId, clientNonce, serverId, serverNonce, keySeed);
		});
		
		std::string serverIdOut = "server-456";
		std::string clientIdOut;
		std::vector<std::uint8_t> clientNonceOut, serverNonceOut, keySeedOut;
		serverTunnel.serverHandshake(serverIdOut, clientIdOut, clientNonceOut, serverNonceOut, keySeedOut);
		
		clientThread.join();
		
		ASSERT(clientIdOut == clientId, "Client ID should match");
		ASSERT(serverId == serverIdOut, "Server ID should match");
		ASSERT(!keySeed.empty(), "Key seed should be generated");
	}
}

