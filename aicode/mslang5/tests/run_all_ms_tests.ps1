param(
  [string]$Cli,
  [switch]$SkipSupportModules
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Normalize-Newline {
  param([string]$Text)
  if ($null -eq $Text) { return '' }
  return ($Text -replace "`r`n", "`n" -replace "`r", "`n")
}

function Get-RepoRoot {
  $root = Join-Path $PSScriptRoot '..'
  return (Resolve-Path $root).Path
}

function Get-RelativePathCompat {
  param([string]$BasePath, [string]$TargetPath)
  $base = (Resolve-Path $BasePath).Path
  $target = (Resolve-Path $TargetPath).Path
  if (-not $base.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
    $base = $base + [System.IO.Path]::DirectorySeparatorChar
  }
  $baseUri = New-Object System.Uri($base)
  $targetUri = New-Object System.Uri($target)
  $relativeUri = $baseUri.MakeRelativeUri($targetUri)
  $relative = [System.Uri]::UnescapeDataString($relativeUri.ToString())
  return $relative.Replace('/', [System.IO.Path]::DirectorySeparatorChar)
}

function Resolve-MapleCli {
  param([string]$RepoRoot, [string]$CliArg)
  if ($CliArg -and $CliArg.Trim().Length -gt 0) {
    $candidate = $CliArg
    if (-not [System.IO.Path]::IsPathRooted($candidate)) {
      $candidate = Join-Path $RepoRoot $candidate
    }
    if (-not (Test-Path $candidate)) {
      throw "maple_cli not found: $candidate"
    }
    return (Resolve-Path $candidate).Path
  }

  $candidates = @(
    (Join-Path $RepoRoot 'build\\Debug\\maple_cli.exe'),
    (Join-Path $RepoRoot 'build\\maple_cli.exe'),
    (Join-Path $RepoRoot 'build\\maple_cli')
  )
  foreach ($p in $candidates) {
    if (Test-Path $p) {
      return (Resolve-Path $p).Path
    }
  }
  throw 'maple_cli not found. Build the project first.'
}

function New-TempCaseDir {
  $root = [System.IO.Path]::GetTempPath()
  $name = 'maple_ms_case_' + [System.Guid]::NewGuid().ToString('N')
  $path = Join-Path $root $name
  [void](New-Item -Path $path -ItemType Directory -Force)
  return $path
}

function Invoke-MapleCli {
  param(
    [string]$CliPath,
    [string]$RepoRoot,
    [string]$ScriptRel
  )

  $scriptAbs = Join-Path $RepoRoot $ScriptRel
  if (-not (Test-Path $scriptAbs)) {
    throw "script not found: $scriptAbs"
  }

  $tmpDir = New-TempCaseDir
  try {
    $entry = Join-Path $tmpDir 'entry.ms'
    Copy-Item -LiteralPath $scriptAbs -Destination $entry -Force

    # Inject module fixtures so imports like util/side/pkg.math resolve for any case location.
    $moduleRoot = Join-Path $RepoRoot 'tests\\scripts\\module'
    if (Test-Path $moduleRoot) {
      Copy-Item -Path (Join-Path $moduleRoot '*') -Destination $tmpDir -Recurse -Force
    }

    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $CliPath
    $psi.Arguments = 'entry.ms'
    $psi.WorkingDirectory = $tmpDir
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true

    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $psi
    [void]$proc.Start()
    $stdout = $proc.StandardOutput.ReadToEnd()
    $stderr = $proc.StandardError.ReadToEnd()
    $proc.WaitForExit()

    $stdout = Normalize-Newline $stdout
    $stderr = Normalize-Newline $stderr
    $combined = $stdout + $stderr

    return [PSCustomObject]@{
      ReturnCode = $proc.ExitCode
      Stdout = $stdout
      Stderr = $stderr
      Combined = $combined
    }
  } finally {
    if (Test-Path $tmpDir) {
      Remove-Item -LiteralPath $tmpDir -Recurse -Force
    }
  }
}

function Parse-ConformanceHeader {
  param([string]$Path)
  $raw = [System.IO.File]::ReadAllText($Path)
  $lines = (Normalize-Newline $raw).Split("`n")
  $meta = @{}
  $stdoutLines = New-Object System.Collections.Generic.List[string]
  $inHeader = $true
  $captureStdout = $false

  foreach ($line in $lines) {
    if ($inHeader -and $line.StartsWith('// @')) {
      $captureStdout = $false
      $payload = $line.Substring(4)
      $idx = $payload.IndexOf(':')
      if ($idx -lt 0) { continue }
      $key = $payload.Substring(0, $idx).Trim()
      $value = $payload.Substring($idx + 1).Trim()
      if ($key -eq 'stdout') {
        $captureStdout = $true
      } elseif ($key -eq 'spec') {
        # multi-value field
      } else {
        $meta[$key] = $value
      }
      continue
    }

    if ($inHeader -and $captureStdout -and $line.StartsWith('//')) {
      $value = $line.Substring(2)
      if ($value.StartsWith(' ')) {
        $value = $value.Substring(1)
      }
      [void]$stdoutLines.Add($value)
      continue
    }

    if ($inHeader) {
      $inHeader = $false
    }
  }

  if ($stdoutLines.Count -gt 0) {
    $meta['stdout'] = (($stdoutLines -join "`n") + "`n")
  }
  return $meta
}

function Is-SupportModuleScript {
  param([string]$RelPath)
  $normalized = $RelPath.Replace([char]92, [char]47).ToLowerInvariant()
  $support = @(
    'tests/scripts/module/cycle_a.ms',
    'tests/scripts/module/cycle_b.ms',
    'tests/scripts/module/side.ms',
    'tests/scripts/module/util.ms',
    'tests/scripts/module/pkg/math.ms'
  )
  return $support -contains $normalized
}

function Is-ExpectedErrorScript {
  param([string]$RelPath, [System.IO.FileInfo]$File)
  $normalized = $RelPath.Replace([char]92, [char]47).ToLowerInvariant()
  if ($normalized -eq 'tests/scripts/module/cycle_a.ms' -or $normalized -eq 'tests/scripts/module/cycle_b.ms') {
    return $true
  }
  $stem = [System.IO.Path]::GetFileNameWithoutExtension($File.Name)
  return ($stem -match '(^|_)error($|_)' -or $stem.EndsWith('_error'))
}

$repoRoot = Get-RepoRoot
$cliPath = Resolve-MapleCli -RepoRoot $repoRoot -CliArg $Cli

$conformanceFiles = Get-ChildItem (Join-Path $repoRoot 'tests\\conformance') -Recurse -File -Filter *.ms | Sort-Object FullName
$scriptFiles = Get-ChildItem (Join-Path $repoRoot 'tests\\scripts') -Recurse -File -Filter *.ms | Sort-Object FullName

$results = New-Object System.Collections.Generic.List[object]
$skipped = New-Object System.Collections.Generic.List[string]

foreach ($file in $conformanceFiles) {
  $rel = (Get-RelativePathCompat -BasePath $repoRoot -TargetPath $file.FullName).Replace([char]92, [char]47)
  $meta = Parse-ConformanceHeader -Path $file.FullName
  $expect = if ($meta.ContainsKey('expect')) { $meta['expect'] } else { '' }

  $run = Invoke-MapleCli -CliPath $cliPath -RepoRoot $repoRoot -ScriptRel $rel
  $passed = $false
  $detail = ''

  if ($expect -eq 'ok') {
    if ($run.ReturnCode -ne 0) {
      $detail = 'expected rc=0'
    } elseif ($meta.ContainsKey('stdout') -and $run.Stdout -ne $meta['stdout']) {
      $detail = 'stdout mismatch'
    } else {
      $passed = $true
    }
  } elseif ($expect -eq 'compile_error' -or $expect -eq 'runtime_error') {
    if ($run.ReturnCode -eq 0) {
      $detail = 'expected non-zero rc'
    } else {
      $phase = if ($meta.ContainsKey('diag.phase')) { $meta['diag.phase'] } else { '' }
      $code = if ($meta.ContainsKey('diag.code')) { $meta['diag.code'] } else { '' }
      $marker = if ($phase -and $code) { "[$phase $code]" } else { '' }
      if ($marker -and -not $run.Combined.Contains($marker) -and -not $run.Combined.Contains($code)) {
        $detail = "missing diag marker/code $marker"
      } elseif ($meta.ContainsKey('diag.message_contains') -and -not $run.Combined.Contains($meta['diag.message_contains'])) {
        $detail = 'missing diag.message_contains'
      } else {
        $passed = $true
      }
    }
  } else {
    $detail = 'unsupported @expect'
  }

  $results.Add([PSCustomObject]@{
    Path = $rel
    Group = 'conformance'
    Expected = $expect
    Passed = $passed
    ReturnCode = $run.ReturnCode
    Detail = $detail
  })
}

foreach ($file in $scriptFiles) {
  $rel = (Get-RelativePathCompat -BasePath $repoRoot -TargetPath $file.FullName).Replace([char]92, [char]47)
  if ($SkipSupportModules -and (Is-SupportModuleScript -RelPath $rel)) {
    [void]$skipped.Add($rel)
    continue
  }

  $expectError = Is-ExpectedErrorScript -RelPath $rel -File $file
  $run = Invoke-MapleCli -CliPath $cliPath -RepoRoot $repoRoot -ScriptRel $rel

  $passed = $false
  $detail = ''
  $expected = if ($expectError) { 'error' } else { 'ok' }

  if ($expectError) {
    if ($run.ReturnCode -ne 0) {
      $passed = $true
    } else {
      $detail = 'expected non-zero rc'
    }
  } else {
    if ($run.ReturnCode -eq 0) {
      $passed = $true
    } else {
      $detail = 'expected rc=0'
    }
  }

  $results.Add([PSCustomObject]@{
    Path = $rel
    Group = 'scripts'
    Expected = $expected
    Passed = $passed
    ReturnCode = $run.ReturnCode
    Detail = $detail
  })
}

$total = $results.Count
$passedCount = ($results | Where-Object { $_.Passed }).Count
$failed = @($results | Where-Object { -not $_.Passed })
$failedCount = $failed.Count
$skipCount = $skipped.Count
$rate = if ($total -gt 0) { [math]::Round(($passedCount * 100.0) / $total, 2) } else { 0.0 }

Write-Output "maple_cli: $cliPath"
Write-Output "total: $total"
Write-Output "passed: $passedCount"
Write-Output "failed: $failedCount"
Write-Output "skipped: $skipCount"
Write-Output "pass_rate: $rate%"

if ($skipCount -gt 0) {
  Write-Output ''
  Write-Output 'skipped support module fixtures:'
  foreach ($s in $skipped) {
    Write-Output "  - $s"
  }
}

if ($failedCount -gt 0) {
  Write-Output ''
  Write-Output 'failed cases:'
  foreach ($f in $failed) {
    Write-Output "  - $($f.Path) [$($f.Group)] expected=$($f.Expected) rc=$($f.ReturnCode) detail=$($f.Detail)"
  }
  exit 1
}

exit 0



