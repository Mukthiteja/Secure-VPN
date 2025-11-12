# Testing Guide

This document provides comprehensive testing procedures for the CustomVPN application.

## Test Suite

The project includes a comprehensive test suite covering all major components.

### Running Tests

```powershell
# Build with tests enabled (default)
.\scripts\build.ps1

# Run test suite
.\scripts\run_tests.ps1
```

### Test Coverage

#### 1. Crypto Tests (`test_crypto.cpp`)

Tests cryptographic operations:
- Key derivation (HKDF-SHA256)
- Encryption/decryption (AES-256-CBC)
- MAC verification (HMAC-SHA256)
- IV randomization
- Corruption detection

**Expected Results:**
- All encryption/decryption operations succeed
- Same plaintext produces different ciphertext (IV randomization)
- Corrupted ciphertext fails MAC verification

#### 2. Tunnel Tests (`test_tunnel.cpp`)

Tests tunnel protocol:
- Frame send/receive
- DATA frame transmission
- HEARTBEAT mechanism
- Handshake protocol

**Expected Results:**
- Frames are transmitted correctly
- Handshake completes successfully
- Heartbeat is received

#### 3. Authentication Tests (`test_auth.cpp`)

Tests authentication system:
- Credential store loading
- Plaintext password verification
- Hashed password verification
- User lookup

**Expected Results:**
- Credential store loads correctly
- Correct passwords are accepted
- Incorrect passwords are rejected
- Nonexistent users are rejected

#### 4. Integration Tests (`test_integration.cpp`)

Tests component integration:
- Server/client construction
- Configuration validation

**Note:** Full integration tests require TLS certificates and are best run manually.

## Manual Testing Procedures

### Test 1: Secure Tunneling

**Objective:** Verify TLS-based secure communication

**Steps:**
1. Start server: `.\build\Release\customvpn.exe --mode=server`
2. Start client: `.\build\Release\customvpn.exe --mode=client`
3. Observe connection establishment

**Expected Results:**
- TLS handshake completes without errors
- Server logs "Session established"
- Client connects successfully
- Data transmission occurs

**Validation:**
- Check server logs for session establishment
- Verify no TLS errors in logs
- Confirm client receives echo response

### Test 2: Encryption

**Objective:** Verify application-layer encryption

**Steps:**
1. Start server with logging
2. Start client
3. Monitor network traffic (optional: use Wireshark)
4. Send test data

**Expected Results:**
- Payloads are encrypted (opaque in network capture)
- Server decrypts and processes data correctly
- Client receives decrypted echo

**Validation:**
- Network capture shows only TLS records (no plaintext)
- Server logs show successful decryption
- Client receives correct echo

### Test 3: Authentication

**Objective:** Verify user authentication

**Test 3.1: Successful Authentication**

**Steps:**
1. Start server: `.\build\Release\customvpn.exe --mode=server --credentials=config\users.json`
2. Start client with correct credentials: `.\build\Release\customvpn.exe --mode=client --username=vpnuser --password=ChangeMe`

**Expected Results:**
- Server logs "User vpnuser authenticated"
- Client connects successfully
- Data transmission works

**Test 3.2: Failed Authentication**

**Steps:**
1. Start server as above
2. Start client with wrong password: `.\build\Release\customvpn.exe --mode=client --username=vpnuser --password=WrongPass`

**Expected Results:**
- Server logs "Authentication failed for user vpnuser"
- Client receives error message
- Connection closes immediately

**Test 3.3: Nonexistent User**

**Steps:**
1. Start server as above
2. Start client with nonexistent user: `.\build\Release\customvpn.exe --mode=client --username=nonexistent --password=any`

**Expected Results:**
- Authentication fails
- Connection closes

### Test 4: Connection Termination

**Objective:** Verify secure connection termination

**Steps:**
1. Establish authenticated connection
2. Client calls `disconnect()`
3. Observe connection closure

**Expected Results:**
- CLOSE frame is sent
- Connection closes gracefully
- No resource leaks

### Test 5: Performance

**Objective:** Measure performance characteristics

**Test 5.1: Connection Establishment**

**Steps:**
1. Measure time from client start to authenticated connection
2. Repeat multiple times
3. Calculate average

**Expected Results:**
- Connection establishment < 1 second
- Consistent timing across runs

**Test 5.2: Throughput**

**Steps:**
1. Send large payloads (1MB, 10MB)
2. Measure transmission time
3. Calculate throughput

**Expected Results:**
- Throughput > 10 MB/s (depends on hardware)
- No significant degradation with size

**Test 5.3: Concurrent Connections**

**Steps:**
1. Start server
2. Connect multiple clients simultaneously
3. Monitor server performance

**Expected Results:**
- Server handles multiple connections
- No performance degradation
- All connections work correctly

### Test 6: Error Handling

**Objective:** Verify error handling and recovery

**Test 6.1: Invalid Certificate**

**Steps:**
1. Use invalid or expired certificate
2. Attempt connection

**Expected Results:**
- Connection fails with appropriate error
- No crash or undefined behavior

**Test 6.2: Network Interruption**

**Steps:**
1. Establish connection
2. Disconnect network
3. Attempt to send data

**Expected Results:**
- Connection detects failure
- Graceful error handling
- No resource leaks

**Test 6.3: Corrupted Data**

**Steps:**
1. Manually corrupt encrypted payload
2. Send to server

**Expected Results:**
- MAC verification fails
- Connection closes or error logged
- No crash

## Stress Testing

### Test 7: Long-Running Connection

**Objective:** Verify stability over time

**Steps:**
1. Establish connection
2. Keep connection alive for extended period (1+ hours)
3. Send periodic heartbeats
4. Monitor for memory leaks or crashes

**Expected Results:**
- Connection remains stable
- No memory leaks
- Performance remains consistent

### Test 8: Rapid Connect/Disconnect

**Objective:** Test connection lifecycle

**Steps:**
1. Rapidly connect and disconnect multiple times
2. Monitor resource usage
3. Check for leaks

**Expected Results:**
- All connections clean up properly
- No resource leaks
- Server remains stable

## Security Testing

### Test 9: Man-in-the-Middle

**Objective:** Verify TLS protection

**Steps:**
1. Attempt to intercept connection
2. Try to decrypt traffic

**Expected Results:**
- Traffic is encrypted
- Cannot decrypt without private keys
- Connection fails if certificates don't match

### Test 10: Replay Attack

**Objective:** Verify nonce-based protection

**Steps:**
1. Capture encrypted frames
2. Attempt to replay

**Expected Results:**
- Replayed frames are rejected or fail MAC verification
- Session keys prevent replay

### Test 11: Brute Force Protection

**Objective:** Verify authentication security

**Steps:**
1. Attempt multiple failed logins
2. Monitor server behavior

**Expected Results:**
- Each failed attempt closes connection
- No information leakage
- Server remains stable

## Regression Testing

After any code changes, run:

1. **Full Test Suite:** `.\scripts\run_tests.ps1`
2. **Manual Tests 1-3:** Basic functionality
3. **Performance Test:** Ensure no degradation

## Test Data

### Sample Payloads

- Small: `{'P','I','N','G'}` (4 bytes)
- Medium: 1KB random data
- Large: 1MB random data
- Edge cases: Empty payload, maximum size

### Test Users

Create test users in `config/users.json`:
- Valid user with known password
- User with hashed password
- User for negative testing

## Continuous Integration

For CI/CD pipelines:

```powershell
# Build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
cmake --build build --config Release

# Run tests
.\build\Release\vpn_tests.exe

# Exit code 0 = success, non-zero = failure
```

## Reporting Issues

When reporting test failures, include:
- Test case name
- Steps to reproduce
- Expected vs actual results
- Environment details (OS, compiler, etc.)
- Logs from server and client
- Network capture (if applicable)

