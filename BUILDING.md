# Building Sample Box

This guide describes the Windows development workflow for the Sample Box standalone application, VST3 plug-in bundle, and tests.

> This project expects a local JUCE checkout at `external/JUCE`. JUCE and generated build files are intentionally not committed to this repository.

## Requirements

Install the following before building:

- Visual Studio 2022 or Visual Studio Build Tools 2022.
- In the Visual Studio Installer, select **Desktop development with C++**.
- Ensure the MSVC C++ build tools and a Windows 10 or Windows 11 SDK are selected.
- CMake.
- Ninja.
- Git.

Use **Developer PowerShell for VS 2022** or **x64 Native Tools Command Prompt for VS 2022**. An ordinary PowerShell window may find `cl.exe` but still lack the complete MSVC and Windows SDK environment.

Typical symptoms of the wrong shell include missing `windows.h`, `filesystem`, `algorithm`, `stdint.h`, or `stddef.h` headers.

## Clone Sample Box

```powershell
git clone https://github.com/schism-studio/sample-box-2.git
Set-Location .\sample-box-2
```

If you already have the repository, open Developer PowerShell and change to its root instead:

```powershell
Set-Location "C:\path\to\sample-box-2"
```

## Install JUCE

Run these commands **from the Sample Box repository root**. They create the required dependency directory and clone JUCE into the path used by the project.

```powershell
New-Item -ItemType Directory -Force .\external | Out-Null
git clone https://github.com/juce-framework/JUCE.git .\external\JUCE
```

If `external\JUCE` already exists and you want to keep it, do not clone again. Verify it instead:

```powershell
Test-Path .\external\JUCE\CMakeLists.txt
git -C .\external\JUCE rev-parse HEAD
```

`Test-Path` must print `True`. Record the hash printed by `rev-parse HEAD` below once this project has been successfully built and tested with that checkout:

```text
Validated JUCE revision: 7aae7d8e8deb8413bb01633d2795ef9974a181c5
```

### Pin JUCE after validation

Do not use `--depth 1` for a reproducible checkout: a shallow clone may not contain the commit you later need to check out. Once you know the validated hash, a new machine can install the exact same JUCE revision with:

```powershell
New-Item -ItemType Directory -Force .\external | Out-Null
git clone https://github.com/juce-framework/JUCE.git .\external\JUCE
git -C .\external\JUCE checkout 7aae7d8e8deb8413bb01633d2795ef9974a181c5
git -C .\external\JUCE rev-parse HEAD
```

The final command must print the same hash recorded in this document.

> Do not add `external/JUCE` to Git. It is a local dependency and should remain ignored.

## Verify the toolchain

From Developer PowerShell or the x64 Native Tools prompt, at the repository root, run:

```powershell
cl
where.exe cmake
where.exe ninja
Test-Path .\external\JUCE\CMakeLists.txt
```

The first three commands should locate the compiler and build tools. The final command must return `True`.

## Configure, build, and test

Configure the project with its Windows Debug preset:

```powershell
cmake --preset windows-debug
```

Build it:

```powershell
cmake --build --preset windows-debug --parallel
```

Run the test suite only after the build completes successfully:

```powershell
ctest --preset windows-debug --output-on-failure
```

For normal source edits, repeat only the build command:

```powershell
cmake --build --preset windows-debug --parallel
```

If Ninja reports `no work to do`, the current build outputs are already up to date.

## Locate build outputs

Generated files are placed under:

```text
build/windows-debug/
```

The expected Debug VST3 bundle location is:

```text
build/windows-debug/src/plugin/SampleBox_VST3_artefacts/Debug/VST3/Sample Box.vst3
```

Confirm it exists:

```powershell
Test-Path ".\build\windows-debug\src\plugin\SampleBox_VST3_artefacts\Debug\VST3\Sample Box.vst3"
```

Find the standalone executable, if that target is enabled:

```powershell
Get-ChildItem -Recurse .\build\windows-debug -File |
  Where-Object { $_.Name -match 'Sample.*Box.*\.exe$' } |
  Select-Object -ExpandProperty FullName
```

Run the returned `.exe` path to launch the standalone app. Load the `.vst3` bundle from a compatible DAW or plug-in host.

## Clean rebuild

Use a clean rebuild only after changing the compiler/toolchain, JUCE checkout, CMake preset, or other build configuration:

```powershell
Remove-Item -Recurse -Force .\build\windows-debug -ErrorAction SilentlyContinue
cmake --preset windows-debug
cmake --build --preset windows-debug --parallel
ctest --preset windows-debug --output-on-failure
```

Do not delete `external/JUCE` unless you deliberately intend to clone or restore JUCE again.

## Troubleshooting

### CMake cannot find JUCE

Check the expected file:

```powershell
Test-Path .\external\JUCE\CMakeLists.txt
```

If it returns `False`, install JUCE again from the repository root:

```powershell
New-Item -ItemType Directory -Force .\external | Out-Null
git clone https://github.com/juce-framework/JUCE.git .\external\JUCE
```

Then rerun configuration:

```powershell
cmake --preset windows-debug
```

### MSVC or Windows headers are missing

Close the terminal. Start **Developer PowerShell for VS 2022** or **x64 Native Tools Command Prompt for VS 2022**, return to the repository root, and rerun the configure and build commands. If `cl` itself is not recognized, install or modify Visual Studio Build Tools to include **Desktop development with C++** and a Windows SDK.

### Existing JUCE directory blocks cloning

If `git clone` reports that `external\JUCE` already exists, verify the existing checkout instead of deleting it:

```powershell
Test-Path .\external\JUCE\CMakeLists.txt
git -C .\external\JUCE status
git -C .\external\JUCE rev-parse HEAD
```
