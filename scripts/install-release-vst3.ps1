[CmdletBinding()]
param(
    [switch]$Build,
    [switch]$KeepAbletonOpen
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$source = Join-Path $repoRoot 'build\windows-release\src\plugin\SampleBox_VST3_artefacts\Release\VST3\Sample Box.vst3'
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
        cmake --preset windows-release
        if ($LASTEXITCODE -ne 0) {
            throw "Release configuration failed with exit code $LASTEXITCODE."
        }

        cmake --build --preset windows-release --parallel
        if ($LASTEXITCODE -ne 0) {
            throw "Release build failed with exit code $LASTEXITCODE."
        }

        ctest --preset windows-release --output-on-failure
        if ($LASTEXITCODE -ne 0) {
            throw "Release tests failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        Pop-Location
    }
}

if (-not (Test-Path -LiteralPath $source -PathType Container)) {
    throw "Release VST3 bundle was not found: $source`nRun this script with -Build, or configure and build the windows-release preset first."
}

if (-not $KeepAbletonOpen) {
    Get-Process | Where-Object { $_.ProcessName -like 'Ableton Live*' } |
        Stop-Process -Force -ErrorAction SilentlyContinue
}

New-Item -ItemType Directory -Force -Path $destinationRoot | Out-Null
Remove-Item -LiteralPath $destination -Recurse -Force -ErrorAction SilentlyContinue
Copy-Item -LiteralPath $source -Destination $destination -Recurse -Force

$installedBinary = Join-Path $destination 'Contents\x86-win\Sample Box.vst3'
if (-not (Test-Path -LiteralPath $installedBinary -PathType Leaf)) {
    throw "Install verification failed: expected VST3 module is missing: $installedBinary"
}

$sourceBinary = Join-Path $source 'Contents\x86-win\Sample Box.vst3'
$sourceHash = (Get-FileHash -LiteralPath $sourceBinary -Algorithm SHA256).Hash
$installedHash = (Get-FileHash -LiteralPath $installedBinary -Algorithm SHA256).Hash

if ($sourceHash -ne $installedHash) {
    throw "Install verification failed: installed module hash differs from the Release build output."
}

Write-Host 'Installed verified Release VST3 bundle:' -ForegroundColor Green
Write-Host $destination
Write-Host 'Restart Ableton Live, then hold Alt while clicking Rescan if Sample Box is not listed.' -ForegroundColor Yellow
