# CustomVpn (C++ + Poco)

Secure VPN application scaffold built with Poco C++ libraries. Day 1 delivers project setup, dependency configuration, build system, and initial architecture.

## Prerequisites
- Windows 10/11, PowerShell
- CMake 3.20+
- A C++17 compiler (MSVC recommended via Visual Studio Build Tools)
- [vcpkg](https://github.com/microsoft/vcpkg) (manifest mode)

## Clone and Bootstrap vcpkg (once)
```powershell
# If you don't have vcpkg:
git clone https://github.com/microsoft/vcpkg.git D:\vcpkg
& D:\vcpkg\bootstrap-vcpkg.bat
```

## Build (Manifest Mode)
```powershell
cd D:\CustomVpn
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCUSTOMVPN_BUILD_TESTS=OFF
cmake --build build --config Release
```

This will install and link Poco automatically via `vcpkg.json`.

## Run
Certificates are required for TLS. Place PEM files in `certs\` (not committed):
- `certs\ca.crt` (CA)
- `certs\server.crt`, `certs\server.key`
- `certs\client.crt`, `certs\client.key`

```powershell
# Server
.\build\Release\customvpn.exe --mode=server

# Client (in another shell)
.\build\Release\customvpn.exe --mode=client
```

Day 2+: On connect, a tunnel handshake occurs and the client sends a framed payload; the server echoes it. From Day 3, payloads are application-encrypted (AES-256-CBC + HMAC-SHA256) on top of TLS.

## Project Layout
- `CMakeLists.txt` root build file
- `vcpkg.json` vcpkg manifest (declares Poco)
- `include\vpn\` public headers (`vpn_client.h`, `vpn_server.h`)
- `src\` sources (`main.cpp`, `vpn_client.cpp`, `vpn_server.cpp`)
- `docs\ARCHITECTURE.md` objectives and high-level design
- `certs\` (ignored) TLS materials (not committed)

## Notes
- TLS provides transport security (encryption/integrity/key exchange). We add an app-layer encryption (AES-256-CBC + HMAC-SHA256) with keys derived via HKDF from a per-session seed exchanged during tunnel handshake.

## Troubleshooting
- If CMake can't find Poco, ensure `-DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake` is passed and vcpkg is bootstrapped.
- For MSVC, run from a "x64 Native Tools Command Prompt for VS" or have Build Tools in PATH.

## Day 3: How to validate encryption
- Build and run server and client as above.
- Expected behavior:
  - Client and server perform extended handshake (nonces + key seed).
  - Client sends ENCRYPTED_DATA; server decrypts, echoes re-encrypted data.
  - Client prints “Received 4 bytes” for the echoed plaintext.
  - Packet capture shows opaque TLS records (no plaintext payload).

# Custom VPN (C++ / Poco)

Secure VPN application scaffold using C++20 and Poco libraries. This repository sets up a client/server foundation with TLS, a basic authentication stub, and scaffolding for tunneling and encryption components.

## Prerequisites (Windows 10/11)

- Visual Studio 2022 with C++ development workload
- CMake 3.20+
- PowerShell
- vcpkg package manager (to install Poco)

## Install vcpkg and Poco

```powershell
cd D:\CustomVpn
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg.exe integrate install
.\vcpkg\vcpkg.exe install poco[netssl,util,crypto]:x64-windows
```

Ensure `VCPKG_ROOT` is set (optional but recommended):

```powershell
$env:VCPKG_ROOT = "D:\CustomVpn\vcpkg"
```

## Configure and Build

```powershell
cd D:\CustomVpn
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
cmake --build build --config Release
```

Binaries:
- `build\Release\vpn-server.exe`
- `build\Release\vpn-client.exe`

## Run (local test)

Start server:
```powershell
.\build\Release\vpn-server.exe
```

In a second terminal, run client:
```powershell
.\build\Release\vpn-client.exe
```

You should see `CustomVPN server ready` from the client.

## Project Layout

```
include/
  vpn/
    common/Logger.h
    core/{Crypto.h, Session.h, Auth.h}
    tunnel/Tunnel.h
src/
  server/main_server.cpp
  client/main_client.cpp
  vpn/
    common/Logger.cpp
    core/{Crypto.cpp, Session.cpp, Auth.cpp}
    tunnel/Tunnel.cpp
```

## Next Steps
- Implement secure tunneling and socket management (Day 2)
- Data encryption and privacy (Day 3)
- User authentication and secure connections (Day 4)
- Testing, docs, and optimization (Day 5)


