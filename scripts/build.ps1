param([ValidateSet('windows-debug', 'windows-release')][string]$Preset = 'windows-debug')
cmake --build --preset $Preset --parallel
