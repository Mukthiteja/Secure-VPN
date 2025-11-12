cmake_minimum_required(VERSION 3.20)

add_executable(vpn_tests
	test_main.cpp
	test_crypto.cpp
	test_tunnel.cpp
	test_auth.cpp
	test_integration.cpp
)

target_link_libraries(vpn_tests
	PRIVATE
	Poco::Foundation
	Poco::Net
	Poco::NetSSL
	Poco::Crypto
	Poco::JSON
	Poco::Util
)

target_include_directories(vpn_tests
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

# Add test executable to CTest
add_test(NAME VPNTests COMMAND vpn_tests)

