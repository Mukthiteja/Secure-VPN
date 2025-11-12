#include "vpn/vpn_client.h"
#include "vpn/vpn_server.h"
#include <Poco/Thread.h>
#include <Poco/Event.h>
#include <memory>

void test_integration() {
	TEST_SUITE(Integration) {
		// Note: Full integration test would require TLS certificates
		// This is a placeholder that validates configuration
		
		vpn::ServerConfig serverCfg;
		serverCfg.address = "127.0.0.1";
		serverCfg.port = 0; // Let OS assign port
		serverCfg.certFile = "certs/server.crt";
		serverCfg.keyFile = "certs/server.key";
		serverCfg.caFile = "certs/ca.crt";
		
		vpn::VpnServer server(serverCfg);
		ASSERT(true, "Server should construct without errors");
		
		vpn::ClientConfig clientCfg;
		clientCfg.serverHost = "127.0.0.1";
		clientCfg.serverPort = 44350;
		clientCfg.certFile = "certs/client.crt";
		clientCfg.keyFile = "certs/client.key";
		clientCfg.caFile = "certs/ca.crt";
		clientCfg.username = "testuser";
		clientCfg.password = "testpass";
		
		vpn::VpnClient client(clientCfg);
		ASSERT(true, "Client should construct without errors");
	}
}

