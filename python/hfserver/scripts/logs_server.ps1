param(
    [string]$LogFile = "run\\hotfix_server.log",
    [int]$Lines = 200
)

$root = Split-Path -Parent $PSScriptRoot
$logPath = Join-Path $root $LogFile

if (-Not (Test-Path $logPath)) {
    Write-Host "Log file not found: $logPath"
    exit 1
}

Get-Content -Path $logPath -Tail $Lines -Wait
