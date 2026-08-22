param([ValidateSet('windows-debug', 'windows-release')][string]$Preset = 'windows-debug')
ctest --preset $Preset --output-on-failure
