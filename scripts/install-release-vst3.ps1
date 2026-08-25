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

$sourceBinary = Get-ChildItem -LiteralPath $source -Recurse -File |
    Where-Object {
        $_.Name -eq 'Sample Box.vst3' -and
        $_.Directory.Name -in @('x86_64-win', 'x86-win')
    } |
    Select-Object -First 1

if ($null -eq $sourceBinary) {
    throw "Release VST3 module was not found inside: $source"
}

$dumpbin = Get-Command dumpbin.exe -ErrorAction SilentlyContinue
if ($null -eq $dumpbin) {
    throw "dumpbin.exe was not found. Run this script from an x64 Visual Studio Developer PowerShell or x64 Native Tools environment."
}

$architecture = & $dumpbin.Source /headers $sourceBinary.FullName 2>&1 |
    Select-String -Pattern 'machine \(x64\)'

if ($null -eq $architecture) {
    throw "Refusing to install a non-x64 VST3 module: $($sourceBinary.FullName)`nBuild from an x64 Visual Studio toolchain; expected dumpbin output containing '8664 machine (x64)'."
}

if (-not $KeepAbletonOpen) {
    Get-Process | Where-Object { $_.ProcessName -like 'Ableton Live*' } |
        Stop-Process -Force -ErrorAction SilentlyContinue
}

New-Item -ItemType Directory -Force -Path $destinationRoot | Out-Null
Remove-Item -LiteralPath $destination -Recurse -Force -ErrorAction SilentlyContinue
Copy-Item -LiteralPath $source -Destination $destination -Recurse -Force

$installedBinary = Join-Path $destination $sourceBinary.FullName.Substring($source.Length).TrimStart('\')

if (-not (Test-Path -LiteralPath $installedBinary -PathType Leaf)) {
    throw "Install verification failed: expected VST3 module is missing: $installedBinary"
}

$sourceHash = (Get-FileHash -LiteralPath $sourceBinary.FullName -Algorithm SHA256).Hash
$installedHash = (Get-FileHash -LiteralPath $installedBinary -Algorithm SHA256).Hash

if ($sourceHash -ne $installedHash) {
    throw "Install verification failed: installed module hash differs from the Release build output."
}

Write-Host 'Installed verified x64 Release VST3 bundle:' -ForegroundColor Green
Write-Host $destination
Write-Host "Module: $installedBinary"
Write-Host 'Restart Ableton Live, then hold Alt while clicking Rescan if Sample Box is not listed.' -ForegroundColor Yellow