add_library(customvpn_core
	${CMAKE_CURRENT_SOURCE_DIR}/vpn_client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vpn_server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/tunnel.cpp
)

target_include_directories(customvpn_core
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(customvpn_core
	PUBLIC
		Poco::Foundation
		Poco::Net
		Poco::Util
		Poco::NetSSL
		Poco::Crypto
		Poco::JSON
)

add_executable(customvpn
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(customvpn
	PRIVATE
		customvpn_core
)

set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_library(vpn_common
	${SRC_ROOT}/vpn/common/Logger.cpp
	${SRC_ROOT}/vpn/core/Crypto.cpp
	${SRC_ROOT}/vpn/core/Session.cpp
	${SRC_ROOT}/vpn/core/Auth.cpp
	${SRC_ROOT}/vpn/tunnel/Tunnel.cpp
)
target_include_directories(vpn_common PUBLIC ${CMAKE_SOURCE_DIR}/include)

find_package(Poco REQUIRED COMPONENTS Net NetSSL Util Crypto Foundation)
target_link_libraries(vpn_common PUBLIC Poco::Net Poco::NetSSL Poco::Util Poco::Crypto Poco::Foundation)

add_executable(vpn_server ${SRC_ROOT}/server/main_server.cpp)
target_link_libraries(vpn_server PRIVATE vpn_common)
set_target_properties(vpn_server PROPERTIES OUTPUT_NAME "vpn-server")

add_executable(vpn_client ${SRC_ROOT}/client/main_client.cpp)
target_link_libraries(vpn_client PRIVATE vpn_common)
set_target_properties(vpn_client PROPERTIES OUTPUT_NAME "vpn-client")


