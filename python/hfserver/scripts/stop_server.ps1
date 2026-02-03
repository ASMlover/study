$root = Split-Path -Parent $PSScriptRoot
$pidPath = Join-Path $root "run\hotfix_server.pid"

if (-Not (Test-Path $pidPath)) {
    Write-Host "PID file not found."
    exit 1
}

$pid = Get-Content -Path $pidPath | Select-Object -First 1
if (-Not $pid) {
    Write-Host "PID file is empty."
    exit 1
}

try {
    Stop-Process -Id $pid -Force -ErrorAction Stop
    Remove-Item -Path $pidPath -Force
    Write-Host "HotfixServer stopped. PID=$pid"
} catch {
    Write-Host "Failed to stop HotfixServer: $($_.Exception.Message)"
    exit 1
}
