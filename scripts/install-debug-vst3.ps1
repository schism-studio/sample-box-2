[CmdletBinding()]
param(
    [switch]$Build,
    [switch]$KeepAbletonOpen
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$source = Join-Path $repoRoot 'build\windows-debug\src\plugin\SampleBox_VST3_artefacts\Debug\VST3\Sample Box.vst3'
$destinationRoot = 'C:\Program Files\Common Files\VST3'
$destination = Join-Path $destinationRoot 'Sample Box.vst3'

$principal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
$isAdministrator = $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdministrator) {
    throw "Administrator PowerShell is required to install into '$destinationRoot'. Reopen PowerShell with 'Run as administrator', then run this script again."
}

if ($Build) {
    Push-Location $repoRoot
    try {
        cmake --build --preset windows-debug --parallel
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        Pop-Location
    }
}

if (-not (Test-Path -LiteralPath $source -PathType Container)) {
    throw "Debug VST3 bundle was not found: $source`nBuild it first, or run this script with -Build."
}

if (-not $KeepAbletonOpen) {
    Get-Process | Where-Object { $_.ProcessName -like 'Ableton Live*' } | Stop-Process -Force -ErrorAction SilentlyContinue
}

New-Item -ItemType Directory -Force -Path $destinationRoot | Out-Null
Remove-Item -LiteralPath $destination -Recurse -Force -ErrorAction SilentlyContinue
Copy-Item -LiteralPath $source -Destination $destination -Recurse -Force

if (-not (Test-Path -LiteralPath $destination -PathType Container)) {
    throw "Install verification failed: $destination was not created."
}

Write-Host "Installed clean Debug VST3 bundle:" -ForegroundColor Green
Write-Host $destination
Write-Host "Restart Ableton Live, then hold Alt while clicking Rescan if Sample Box is not listed." -ForegroundColor Yellow
