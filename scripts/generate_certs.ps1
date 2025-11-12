# PowerShell script to generate self-signed certificates for VPN testing
# Requires OpenSSL (install via chocolatey: choco install openssl)

param(
    [string]$OutputDir = "certs",
    [string]$CAKeySize = "4096",
    [string]$CertKeySize = "2048",
    [int]$DaysValid = 365
)

$ErrorActionPreference = "Stop"

# Check if OpenSSL is available
$openssl = Get-Command openssl -ErrorAction SilentlyContinue
if (-not $openssl) {
    Write-Host "Error: OpenSSL not found. Please install OpenSSL:" -ForegroundColor Red
    Write-Host "  choco install openssl" -ForegroundColor Yellow
    Write-Host "  Or download from: https://slproweb.com/products/Win32OpenSSL.html" -ForegroundColor Yellow
    exit 1
}

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
    Write-Host "Created directory: $OutputDir" -ForegroundColor Green
}

Write-Host "Generating CA private key..." -ForegroundColor Cyan
& openssl genrsa -out "$OutputDir\ca.key" $CAKeySize

Write-Host "Generating CA certificate..." -ForegroundColor Cyan
& openssl req -new -x509 -days $DaysValid -key "$OutputDir\ca.key" -out "$OutputDir\ca.crt" `
    -subj "/C=US/ST=State/L=City/O=CustomVPN/OU=CA/CN=CustomVPN-CA"

Write-Host "Generating server private key..." -ForegroundColor Cyan
& openssl genrsa -out "$OutputDir\server.key" $CertKeySize

Write-Host "Generating server certificate request..." -ForegroundColor Cyan
& openssl req -new -key "$OutputDir\server.key" -out "$OutputDir\server.csr" `
    -subj "/C=US/ST=State/L=City/O=CustomVPN/OU=Server/CN=server"

Write-Host "Signing server certificate with CA..." -ForegroundColor Cyan
# Create temporary config file for server extensions
$serverExtFile = "$OutputDir\server_ext.conf"
@"
[v3_server]
subjectAltName=DNS:localhost,DNS:server,IP:127.0.0.1
"@ | Out-File -FilePath $serverExtFile -Encoding ASCII
& openssl x509 -req -days $DaysValid -in "$OutputDir\server.csr" -CA "$OutputDir\ca.crt" `
    -CAkey "$OutputDir\ca.key" -CAcreateserial -out "$OutputDir\server.crt" `
    -extensions v3_server -extfile $serverExtFile

Write-Host "Generating client private key..." -ForegroundColor Cyan
& openssl genrsa -out "$OutputDir\client.key" $CertKeySize

Write-Host "Generating client certificate request..." -ForegroundColor Cyan
& openssl req -new -key "$OutputDir\client.key" -out "$OutputDir\client.csr" `
    -subj "/C=US/ST=State/L=City/O=CustomVPN/OU=Client/CN=client"

Write-Host "Signing client certificate with CA..." -ForegroundColor Cyan
# Create temporary config file for client extensions
$clientExtFile = "$OutputDir\client_ext.conf"
@"
[v3_client]
extendedKeyUsage=clientAuth
"@ | Out-File -FilePath $clientExtFile -Encoding ASCII
& openssl x509 -req -days $DaysValid -in "$OutputDir\client.csr" -CA "$OutputDir\ca.crt" `
    -CAkey "$OutputDir\ca.key" -CAcreateserial -out "$OutputDir\client.crt" `
    -extensions v3_client -extfile $clientExtFile

# Cleanup temporary files
Remove-Item "$OutputDir\server.csr", "$OutputDir\client.csr", $serverExtFile, $clientExtFile -ErrorAction SilentlyContinue

Write-Host "`nCertificate generation complete!" -ForegroundColor Green
Write-Host "Files created in: $OutputDir" -ForegroundColor Green
Write-Host "  - ca.crt (CA certificate)" -ForegroundColor White
Write-Host "  - ca.key (CA private key)" -ForegroundColor White
Write-Host "  - server.crt (Server certificate)" -ForegroundColor White
Write-Host "  - server.key (Server private key)" -ForegroundColor White
Write-Host "  - client.crt (Client certificate)" -ForegroundColor White
Write-Host "  - client.key (Client private key)" -ForegroundColor White
Write-Host "`nWARNING: These are self-signed certificates for testing only!" -ForegroundColor Yellow
Write-Host "Do not use in production environments." -ForegroundColor Yellow

