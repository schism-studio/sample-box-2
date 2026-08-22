param([ValidateSet('windows-release')][string]$Preset = 'windows-release')
cmake --build --preset $Preset --parallel
Write-Host 'Packaging will be added after the standalone application has real assets and runtime dependencies.'
