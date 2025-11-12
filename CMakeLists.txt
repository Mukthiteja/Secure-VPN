cmake_minimum_required(VERSION 3.20)

project(CustomVpn
	VERSION 0.1.0
	DESCRIPTION "Secure VPN application using Poco C++ libraries"
	LANGUAGES CXX)

# Options
option(CUSTOMVPN_BUILD_TESTS "Build tests" ON)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prefer modern MSVC warnings if using MSVC
if(MSVC)
	add_compile_options(/W4 /permissive-)
else()
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# vcpkg manifest mode is recommended. If using classic mode, ensure toolchain is set by user:
# cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake
# Enable find_package to work with vcpkg-installed packages

find_package(Poco CONFIG REQUIRED COMPONENTS Foundation Net Util NetSSL Crypto JSON)

add_subdirectory(src)

if(CUSTOMVPN_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()


