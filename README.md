# CustomVPN - Secure VPN Application

A secure VPN application built with C++ and Poco libraries, implementing encryption, secure tunneling, and user authentication.

## Features

- **Secure Tunneling**: TLS-based secure communication channel
- **Application-Layer Encryption**: AES-256-CBC with HMAC-SHA256 for data privacy
- **User Authentication**: Username/password authentication with credential store
- **Session Management**: Per-session key derivation with forward secrecy
- **Frame-Based Protocol**: Custom protocol for flexible message handling
- **Performance Optimized**: Efficient polling, buffer management, and resource handling

## Prerequisites

- **Windows 10/11** with PowerShell
- **CMake 3.20+**
- **C++17 compiler** (MSVC 2019+ recommended)
- **vcpkg** package manager
- **OpenSSL** (for certificate generation, install via Chocolatey: `choco install openssl`)

## Quick Start

### 1. Install vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg.git D:\vcpkg
& D:\vcpkg\bootstrap-vcpkg.bat
$env:VCPKG_ROOT = "D:\vcpkg"
```

### 2. Generate Certificates

```powershell
.\scripts\generate_certs.ps1
```

This creates self-signed certificates in `certs\` directory for testing.

### 3. Build

```powershell
.\scripts\build.ps1
```

Or manually:
```powershell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build --config Release
```

### 4. Run

**Server:**
```powershell
.\build\Release\customvpn.exe --mode=server --credentials=config\users.json
```

**Client (in another terminal):**
```powershell
.\build\Release\customvpn.exe --mode=client --username=vpnuser --password=ChangeMe
```

## Project Structure

```
CustomVpn/
├── include/vpn/          # Public headers
│   ├── vpn_server.h      # Server interface
│   ├── vpn_client.h      # Client interface
│   ├── tunnel.h          # Tunnel protocol
│   ├── crypto.h          # Encryption utilities
│   └── auth.h            # Authentication
├── src/                  # Implementation
│   ├── vpn_server.cpp
│   ├── vpn_client.cpp
│   ├── tunnel.cpp
│   ├── crypto.cpp
│   ├── auth.cpp
│   └── main.cpp
├── tests/                # Test suite
│   ├── test_crypto.cpp
│   ├── test_tunnel.cpp
│   ├── test_auth.cpp
│   └── test_integration.cpp
├── scripts/              # Helper scripts
│   ├── generate_certs.ps1
│   ├── build.ps1
│   └── run_tests.ps1
├── config/               # Configuration
│   └── users.json        # User credentials
├── certs/                # TLS certificates (not committed)
├── docs/                 # Documentation
│   ├── ARCHITECTURE.md
│   └── DESIGN_DECISIONS.md
└── CMakeLists.txt
```

## Configuration

### User Credentials

Edit `config/users.json` to add users:

```json
{
  "users": [
    {
      "username": "vpnuser",
      "password": "ChangeMe"
    },
    {
      "username": "hasheduser",
      "salt": "base64-encoded-salt",
      "hash": "base64-encoded-sha256-hash"
    }
  ]
}
```

For production, use `salt` and `hash` fields instead of plaintext passwords.

### Server Configuration

Default server configuration:
- Address: `0.0.0.0` (all interfaces)
- Port: `44350`
- Certificate: `certs/server.crt`
- Private Key: `certs/server.key`
- CA: `certs/ca.crt`

### Client Configuration

Default client configuration:
- Server: `localhost:44350`
- Certificate: `certs/client.crt`
- Private Key: `certs/client.key`
- CA: `certs/ca.crt`

## Testing

### Run Test Suite

```powershell
.\scripts\run_tests.ps1
```

Or manually:
```powershell
.\build\Release\vpn_tests.exe
```

### Manual Testing

1. **Test Secure Tunneling:**
   - Start server and client
   - Verify TLS handshake completes
   - Check that data is transmitted securely

2. **Test Encryption:**
   - Monitor network traffic (Wireshark)
   - Verify payloads are encrypted
   - Confirm decryption works correctly

3. **Test Authentication:**
   - Try correct credentials (should succeed)
   - Try incorrect credentials (should fail)
   - Verify connection closes on auth failure

4. **Test Performance:**
   - Measure connection establishment time
   - Test throughput with large payloads
   - Monitor CPU and memory usage

## Security Features

### Transport Security
- TLS 1.2+ with strong cipher suites
- Optional mutual TLS authentication
- Certificate validation

### Application Security
- AES-256-CBC encryption
- HMAC-SHA256 for message integrity
- Per-session key derivation (HKDF-SHA256)
- Forward secrecy through unique session keys

### Authentication
- Encrypted credential transmission
- Support for salted password hashes
- Secure connection termination on failure

## Performance Optimizations

- **Efficient Polling**: Optimized timeout values (100ms for data, 1ms for heartbeat)
- **Thread Management**: Proper thread yielding to prevent CPU spinning
- **Memory Management**: RAII, move semantics, buffer reuse
- **Connection Pooling**: Server uses thread pool for concurrent connections
- **Crypto Acceleration**: Leverages OpenSSL hardware acceleration

## Troubleshooting

### Build Issues

**CMake can't find Poco:**
- Ensure `CMAKE_TOOLCHAIN_FILE` points to vcpkg
- Verify vcpkg is bootstrapped: `.\vcpkg\bootstrap-vcpkg.bat`
- Check `vcpkg.json` includes `poco` dependency

**Certificate errors:**
- Generate certificates: `.\scripts\generate_certs.ps1`
- Verify certificate files exist in `certs\` directory
- Check certificate validity dates

### Runtime Issues

**Connection refused:**
- Verify server is running
- Check firewall settings
- Ensure port 44350 is not blocked

**Authentication failed:**
- Verify user exists in `config/users.json`
- Check username/password spelling
- Ensure credential file path is correct

**TLS handshake errors:**
- Verify certificates are valid
- Check CA certificate matches server/client certs
- Ensure certificate files are readable

## Documentation

- [Architecture Documentation](docs/ARCHITECTURE.md) - System design and components
- [Design Decisions](docs/DESIGN_DECISIONS.md) - Security measures and optimizations

## Development Roadmap

### Completed (Days 1-5)
- ✅ Project setup and VPN design
- ✅ Secure tunneling and socket management
- ✅ Encryption and data privacy
- ✅ User authentication and secure connections
- ✅ Testing, documentation, and optimization

### Future Enhancements
- TUN/TAP interface integration for actual VPN routing
- UDP support for better performance
- Compression support
- Additional authentication methods (certificates, tokens)
- Perfect forward secrecy with ephemeral keys
- Metrics and monitoring
- Rate limiting

## License

This project is provided as-is for educational and development purposes.

## Contributing

This is a learning project. Suggestions and improvements are welcome!
