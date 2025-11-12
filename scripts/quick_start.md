# Quick Start Guide - Running CustomVPN

## Current Status
✅ Certificates generated  
✅ Configuration files ready  
⚠️  Project needs to be built first

## To Build and Run:

### Option 1: Using Visual Studio Developer Command Prompt (Recommended)

1. **Open "x64 Native Tools Command Prompt for VS 2019"** (or VS 2022)
   - Start Menu → Visual Studio 2019 → x64 Native Tools Command Prompt

2. **Navigate to project:**
   ```cmd
   cd D:\CustomVpn
   ```

3. **Build the project:**
   ```cmd
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
   cmake --build build --config Release
   ```

4. **Run Server (Terminal 1):**
   ```cmd
   build\Release\customvpn.exe --mode=server --credentials=config\users.json
   ```

5. **Run Client (Terminal 2):**
   ```cmd
   build\Release\customvpn.exe --mode=client --username=vpnuser --password=ChangeMe
   ```

### Option 2: Using PowerShell with VS Environment

```powershell
# Initialize VS environment
& "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

# Build
cd D:\CustomVpn
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build --config Release

# Run (in separate terminals)
.\build\Release\customvpn.exe --mode=server --credentials=config\users.json
.\build\Release\customvpn.exe --mode=client --username=vpnuser --password=ChangeMe
```

## Expected Output

**Server:**
```
VPN server started
Session established serverId=<uuid> clientId=<uuid>
User vpnuser authenticated
```

**Client:**
```
Connected to VPN server
Received 4 bytes
Disconnected from VPN server
```

## Troubleshooting

If build fails with vcpkg errors:
1. Update vcpkg: `cd D:\vcpkg && git pull`
2. Try installing Poco manually: `.\vcpkg.exe install poco:x64-windows`
3. Check Visual Studio C++ tools are installed

