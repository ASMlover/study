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
$pidPath = Join-Path $root "run\hotfix_server.pid"
$runDir = Split-Path -Parent $pidPath

New-Item -ItemType Directory -Path $runDir -Force | Out-Null

if (Test-Path $pidPath) {
    $existingPid = Get-Content -Path $pidPath | Select-Object -First 1
    if ($existingPid) {
        $proc = Get-Process -Id $existingPid -ErrorAction SilentlyContinue
        if ($proc) {
            Write-Host "HotfixServer is already running. PID=$existingPid"
            exit 0
        }
    }
}

$args = @(
    "hotfix_server.py",
    "--host", $BindHost,
    "--port", $Port,
    "--upload-dir", $UploadDir,
    "--static-dir", $StaticDir,
    "--max-files", $MaxFiles,
    "--config", $Config,
    "--log-file", $LogFile
)

if ($TrustXForwardedFor) {
    $args += "--trust-x-forwarded-for"
}

$proc = Start-Process -FilePath "python" -ArgumentList $args -PassThru -WorkingDirectory $root
$proc.Id | Set-Content -Path $pidPath
Write-Host "HotfixServer started. PID=$($proc.Id)"
