## CustomVpn - Architecture and Objectives

### Objectives
- **Secure tunneling**: Provide an encrypted channel over untrusted networks between client and server.
- **Encryption**: Leverage TLS (via Poco::Net::NetSSL) to protect confidentiality and integrity.
- **Data privacy**: Ensure traffic is authenticated, encrypted, and tamper-evident.
- **Session management**: Establish authenticated sessions with mutual TLS optionality.
- **Extensibility**: Clean modular structure for future TUN/TAP integration and auth backends.

### High-Level Architecture
- **Client** (`vpn::VpnClient`)
  - Establishes TLS connection to server using `SecureStreamSocket`.
  - Verifies server certificate (configurable) and presents client certificate if required.
  - Sends/receives framed payloads (placeholder: simple echo).
  - Future: Integrate with a virtual network interface (e.g., Wintun/TAP on Windows).

- **Server** (`vpn::VpnServer`)
  - Listens on TLS (`SecureServerSocket`) and accepts client connections.
  - Optionally enforces mutual TLS client authentication.
  - Manages per-connection session state.
  - Future: Policy enforcement, routing, multiplexing, compression.

- **Control vs Data Plane**
  - Initially combined on the same TLS connection.
  - Future: Separate control channel (auth, keepalive) and data channel (tunneling).

### Security Design
- **Transport security**: TLS 1.2+ via Poco::Net::Context with strong ciphers.
- **Key exchange**: Handled by TLS (ECDHE). No custom key exchange logic required.
- **Authentication**:
  - Server authentication is required by default.
  - Client authentication via mutual TLS is supported and recommended for production.
- **Integrity**: TLS provides authenticated encryption (AEAD) ensuring integrity and replay protection.
- **Certificates**: PEM files for CA, server, client. Place in `certs/` (not committed).

### Protocol Outline (Initial)
- Handshake: Standard TLS handshake.
- Application protocol:
  - On connect, server sends `OK VPN-HELLO`.
  - Client can send framed payloads (placeholder: echo).
  - Heartbeat/keepalive to be added.

### Component Responsibilities
- `vpn::VpnServer`
  - TLS context setup (CA, cert, key, client auth policy)
  - Connection acceptance and lifecycle management
  - Dispatch to handlers for control/data messages
- `vpn::VpnClient`
  - TLS context setup and server verification
  - Connection lifecycle and send/receive API

### Configuration
- `ServerConfig`:
  - `address`, `port`, `certFile`, `keyFile`, `caFile`, `requireClientAuth`
- `ClientConfig`:
  - `serverHost`, `serverPort`, `certFile`, `keyFile`, `caFile`, `verifyServer`

### Future Work
- **Tunneling**: Integrate TUN/TAP (Windows: Wintun or TAP-Windows).
- **User authentication**: Beyond mTLS, add credential/token-based auth.
- **Multiplexing**: Multiple streams over one TLS connection.
- **Compression and QoS**: Optional.
- **Telemetry**: Metrics, structured logs, and tracing.

# Architecture and Security Objectives

## Objectives
- Secure tunneling over untrusted networks
- Strong encryption for data-in-transit
- User authentication and session management
- Defense-in-depth, minimal attack surface

## High-level Components
- Client
  - Control channel over TLS to authenticate and negotiate session
  - Data channel (initially TCP TLS; future: packet tunnel)
- Server
  - TLS termination, authentication, session issuance
  - Data relay and policy enforcement
- Common
  - Crypto utilities
  - Logger
  - Session model

## Channels
- Control channel: TLS over TCP (Poco NetSSL)
- Data channel: Phase 1: TLS over TCP (stream-based); Phase 2: add TUN/TAP-based tunneling

## Key Exchange and Sessions
- Phase 1: TLS-provided key exchange; session ID plus derived key material for data channel
- Phase 2: Bind session to device identity and ephemeral keys

## Authentication
- Phase 1: Token-based (scaffold)
- Phase 2: Username/password + MFA and revocation

## Risks and Mitigations
- Certificate trust: pinning and strict verification in production
- Replay: session IDs are random and short-lived
- DoS: connection rate limiting at server


