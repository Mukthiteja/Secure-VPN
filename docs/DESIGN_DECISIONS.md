# Design Decisions and Security Measures

## Overview

This document outlines the key design decisions, security measures, challenges faced, and optimization strategies implemented in the CustomVPN application.

## Architecture Decisions

### 1. Layered Security Model

**Decision**: Implement multiple layers of security (TLS transport + application-layer encryption)

**Rationale**:
- Defense in depth: Even if TLS is compromised, application data remains encrypted
- Compliance: Meets requirements for sensitive data transmission
- Flexibility: Allows for future protocol extensions without changing transport layer

**Implementation**:
- TLS 1.2+ for transport security (via Poco NetSSL)
- AES-256-CBC for application-layer encryption
- HMAC-SHA256 for message authentication

### 2. Frame-Based Protocol

**Decision**: Use a custom frame-based protocol over TLS

**Rationale**:
- Clear message boundaries without relying on TLS record boundaries
- Supports multiple message types (DATA, AUTH, HEARTBEAT, etc.)
- Enables future protocol extensions

**Frame Format**:
```
[Length: 4 bytes][Type: 1 byte][Payload: variable]
```

### 3. Session Key Derivation

**Decision**: Derive session keys using HKDF-SHA256 from shared secret + nonces

**Rationale**:
- Forward secrecy: Each session uses unique keys
- Nonce-based: Prevents replay attacks
- Standard algorithm: HKDF is well-vetted and widely used

**Process**:
1. Client generates random nonce
2. Server generates random nonce and key seed
3. Both sides derive encryption and MAC keys using HKDF

### 4. Authentication Flow

**Decision**: Authenticate after TLS handshake but before data transmission

**Rationale**:
- Credentials are encrypted by session crypto (derived from TLS)
- Prevents unauthorized access even if TLS is compromised
- Allows for flexible authentication mechanisms

**Flow**:
1. TLS handshake establishes secure channel
2. Session key derivation
3. Client sends encrypted credentials
4. Server verifies and responds with AUTH_RESULT
5. Data transmission begins only after successful auth

## Security Measures

### 1. Encryption

- **Transport Layer**: TLS 1.2+ with strong cipher suites
- **Application Layer**: AES-256-CBC with random IV per message
- **Key Derivation**: HKDF-SHA256 with unique nonces per session

### 2. Authentication

- **Password Storage**: Supports both plaintext (testing) and salted SHA-256 hashes (production)
- **Credential Transmission**: Encrypted using session crypto before transmission
- **Verification**: Server validates credentials before allowing data flow

### 3. Message Integrity

- **HMAC-SHA256**: Every encrypted message includes MAC for integrity verification
- **Decrypt-then-Verify**: MAC is verified before decryption to prevent timing attacks

### 4. Connection Security

- **Mutual TLS**: Optional client certificate verification
- **Secure Termination**: CLOSE frame for graceful disconnection
- **Heartbeat**: Maintains connection liveness and detects dead connections

### 5. Error Handling

- **Fail-Safe**: Authentication failures immediately close connection
- **Crypto Errors**: Decryption failures terminate session to prevent resource waste
- **Timeout Protection**: All network operations have timeouts

## Challenges Faced

### 1. Certificate Management

**Challenge**: Setting up TLS certificates for testing

**Solution**: Created PowerShell script (`scripts/generate_certs.ps1`) to automate certificate generation using OpenSSL

### 2. Frame Synchronization

**Challenge**: Ensuring reliable frame boundaries over TCP stream

**Solution**: Fixed-length header (4 bytes) followed by type byte, then variable payload. Receiver reads header first to know payload size.

### 3. Session Key Exchange

**Challenge**: Securely exchanging keys without additional round trips

**Solution**: Server generates key seed and sends it in HELLO_ACK. Both sides derive keys deterministically from seed + nonces.

### 4. Authentication Timing

**Challenge**: When to authenticate - before or after session establishment?

**Solution**: After TLS handshake but before data transmission. This ensures credentials are protected by both TLS and session crypto.

### 5. Performance vs Security

**Challenge**: Balancing encryption overhead with performance

**Solution**: 
- Use efficient algorithms (AES hardware acceleration, SHA-256)
- Optimize polling timeouts (100ms for data, 1ms for heartbeat)
- Thread yielding to prevent tight loops

## Optimization Strategies

### 1. Network I/O

- **Timeout Optimization**: Shorter timeouts (100ms) for data polling, minimal (1ms) for heartbeat checks
- **Non-blocking Checks**: Heartbeat and legacy DATA use minimal timeouts to avoid blocking
- **Thread Yielding**: Brief yield in idle loop to prevent CPU spinning

### 2. Memory Management

- **RAII**: All resources managed through smart pointers and RAII
- **Buffer Reuse**: Tunnel frames use vector with reserve for efficient memory usage
- **Move Semantics**: Use move semantics where possible to avoid copies

### 3. Cryptographic Operations

- **Hardware Acceleration**: Rely on Poco's use of OpenSSL, which leverages CPU crypto instructions
- **Key Caching**: Session keys cached per connection, not regenerated per message
- **Efficient MAC**: HMAC-SHA256 is fast and provides strong security

### 4. Connection Management

- **Thread Pool**: Server uses Poco's thread pool for concurrent connections
- **Connection Limits**: Configurable max threads and queued connections
- **Graceful Shutdown**: Proper cleanup on connection termination

### 5. Logging

- **Structured Logging**: Use Poco Logger with appropriate log levels
- **Minimal Overhead**: Log only important events (auth success/failure, errors)
- **No Sensitive Data**: Never log passwords or encryption keys

## Future Enhancements

### 1. Performance

- Connection pooling for client
- Zero-copy buffer management
- Async I/O for better scalability

### 2. Security

- Support for additional authentication methods (certificates, tokens)
- Perfect forward secrecy with ephemeral keys
- Rate limiting for authentication attempts

### 3. Features

- TUN/TAP interface integration for actual VPN routing
- Compression support
- Multi-protocol support (UDP, etc.)

### 4. Monitoring

- Metrics collection (connection count, throughput, latency)
- Health check endpoints
- Performance profiling tools

## Conclusion

The CustomVPN application implements a secure, layered architecture with strong encryption, authentication, and integrity protection. The design prioritizes security while maintaining reasonable performance through careful optimization of critical paths. The modular architecture allows for future enhancements without major refactoring.

