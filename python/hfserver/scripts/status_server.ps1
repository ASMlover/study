$root = Split-Path -Parent $PSScriptRoot
$pidPath = Join-Path $root "run\\hotfix_server.pid"

if (-Not (Test-Path $pidPath)) {
    Write-Host "HotfixServer status: stopped (pid file missing)"
    exit 1
}

$pid = Get-Content -Path $pidPath | Select-Object -First 1
if (-Not $pid) {
    Write-Host "HotfixServer status: unknown (pid file empty)"
    exit 1
}

$proc = Get-Process -Id $pid -ErrorAction SilentlyContinue
if ($proc) {
    Write-Host "HotfixServer status: running (PID=$pid)"
    exit 0
}

Write-Host "HotfixServer status: stopped (stale pid file)"
exit 1
