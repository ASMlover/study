param(
    [string]$BindHost = "0.0.0.0",
    [int]$Port = 8000,
    [string]$UploadDir = "uploads",
    [string]$StaticDir = "static",
    [int]$MaxFiles = 20,
    [string]$Config = "config.json",
    [string]$LogFile = "run\\hotfix_server.log",
    [switch]$TrustXForwardedFor
)

$root = Split-Path -Parent $PSScriptRoot
& (Join-Path $root "scripts\\stop_server.ps1") | Out-Null
Start-Sleep -Milliseconds 300
& (Join-Path $root "scripts\\start_server.ps1") `
    -BindHost $BindHost `
    -Port $Port `
    -UploadDir $UploadDir `
    -StaticDir $StaticDir `
    -MaxFiles $MaxFiles `
    -Config $Config `
    -LogFile $LogFile `
    -TrustXForwardedFor:$TrustXForwardedFor
