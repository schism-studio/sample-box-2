@echo off
setlocal EnableDelayedExpansion

rem ===========================================================================
rem Sample Box - one-step Windows build.
rem
rem Run this from any shell, or just double-click it. It imports the Visual
rem Studio 2022 build environment itself, so it does NOT need to be run from
rem "Developer PowerShell for VS 2022".
rem
rem That bootstrap is the point of this script. An ordinary shell can find
rem cl.exe on PATH while still lacking the MSVC and Windows SDK include
rem directories, and the result is a wall of confusing errors about missing
rem windows.h, filesystem, algorithm, stdint.h or stddef.h - none of which
rem are actually the problem. Importing vcvars64.bat also puts the CMake and
rem Ninja that ship with Visual Studio on PATH.
rem
rem Usage:
rem   scripts\build.bat                     Debug build, then run tests
rem   scripts\build.bat release             Release build, then run tests
rem   scripts\build.bat notest              Debug build, skip tests
rem   scripts\build.bat clean               Wipe the build dir, then rebuild
rem   scripts\build.bat release install     Release build, then copy the VST3
rem                                         into the system VST3 folder
rem
rem Arguments may be combined in any order.
rem ===========================================================================

set "PRESET=windows-debug"
set "CONFIG=Debug"
set "RUN_TESTS=1"
set "DO_CLEAN=0"
set "DO_INSTALL=0"

:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="debug"    ( set "PRESET=windows-debug"   & set "CONFIG=Debug"   & shift & goto parse_args )
if /i "%~1"=="release"  ( set "PRESET=windows-release" & set "CONFIG=Release" & shift & goto parse_args )
if /i "%~1"=="notest"   ( set "RUN_TESTS=0"   & shift & goto parse_args )
if /i "%~1"=="clean"    ( set "DO_CLEAN=1"    & shift & goto parse_args )
if /i "%~1"=="install"  ( set "DO_INSTALL=1"  & shift & goto parse_args )
echo [ERROR] Unrecognised argument: %~1
echo         Valid arguments: debug release notest clean install
goto fail
:args_done

rem The repository root is the parent of this script's directory.
set "ROOT=%~dp0.."
pushd "%ROOT%" || goto fail
set "ROOT=%CD%"

echo ===========================================================================
echo  Sample Box build
echo  Repository : %ROOT%
echo  Preset     : %PRESET%  (%CONFIG%)
echo ===========================================================================
echo.

rem ---------------------------------------------------------------------------
rem 1. Visual Studio environment
rem ---------------------------------------------------------------------------
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if defined VSCMD_VER goto vs_ready
if defined VCINSTALLDIR goto vs_ready

echo [1/5] Importing the Visual Studio build environment...

if not exist "%VSWHERE%" (
    echo.
    echo [ERROR] Could not find vswhere.exe at:
    echo         %VSWHERE%
    echo.
    echo         That file ships with every Visual Studio 2022 install, so this
    echo         usually means Visual Studio 2022 or the Build Tools are not
    echo         installed. Install "Desktop development with C++" plus a
    echo         Windows 10 or 11 SDK, then run this script again.
    goto fail
)

set "VSPATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do set "VSPATH=%%i"

if not defined VSPATH (
    echo.
    echo [ERROR] Visual Studio is installed, but no instance carries the C++
    echo         toolchain ^(Microsoft.VisualStudio.Component.VC.Tools.x86.x64^).
    echo.
    echo         Open the Visual Studio Installer, choose Modify, and enable
    echo         "Desktop development with C++" together with a Windows 10 or
    echo         11 SDK.
    goto fail
)

set "VCVARS=%VSPATH%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
    echo.
    echo [ERROR] Found Visual Studio at "%VSPATH%" but not its x64 environment
    echo         script at:
    echo         %VCVARS%
    goto fail
)

call "%VCVARS%" >nul
if errorlevel 1 (
    echo [ERROR] vcvars64.bat failed. Run it directly to see why:
    echo         "%VCVARS%"
    goto fail
)
echo       Using: %VSPATH%
goto vs_checked

:vs_ready
echo [1/5] Already inside a Visual Studio developer environment - reusing it.

:vs_checked
echo.

rem ---------------------------------------------------------------------------
rem 2. Tool and dependency checks
rem ---------------------------------------------------------------------------
echo [2/5] Checking tools and dependencies...

where cmake >nul 2>&1 || (
    echo [ERROR] cmake was not found on PATH, even after importing the Visual
    echo         Studio environment. Install CMake, or enable the "C++ CMake
    echo         tools for Windows" component in the Visual Studio Installer.
    goto fail
)
where ninja >nul 2>&1 || (
    echo [ERROR] ninja was not found on PATH. Enable the "C++ CMake tools for
    echo         Windows" component in the Visual Studio Installer, which
    echo         supplies both CMake and Ninja.
    goto fail
)

if not exist "%ROOT%\external\JUCE\CMakeLists.txt" (
    echo.
    echo [ERROR] JUCE is missing. Expected to find:
    echo         %ROOT%\external\JUCE\CMakeLists.txt
    echo.
    echo         JUCE is a local dependency and is deliberately not committed.
    echo         Install the validated revision from the repository root:
    echo.
    echo           git clone https://github.com/juce-framework/JUCE.git external\JUCE
    echo           git -C external\JUCE checkout 7aae7d8e8deb8413bb01633d2795ef9974a181c5
    echo.
    echo         See BUILDING.md for the full explanation.
    goto fail
)

for /f "usebackq tokens=*" %%h in (`git -C "%ROOT%\external\JUCE" rev-parse HEAD 2^>nul`) do set "JUCEREV=%%h"
if defined JUCEREV (
    echo       JUCE revision: !JUCEREV!
    if /i not "!JUCEREV!"=="7aae7d8e8deb8413bb01633d2795ef9974a181c5" (
        echo       [WARNING] That is not the revision this project was validated
        echo                 against ^(7aae7d8e8deb8413bb01633d2795ef9974a181c5^).
        echo                 The build may still work, but if it fails in JUCE
        echo                 itself rather than in src\, suspect this first.
    )
)
echo.

rem ---------------------------------------------------------------------------
rem 3. Configure
rem ---------------------------------------------------------------------------
set "BUILDDIR=%ROOT%\build\%PRESET%"

if "%DO_CLEAN%"=="1" (
    if exist "%BUILDDIR%" (
        echo [3/5] Removing "%BUILDDIR%"...
        rmdir /s /q "%BUILDDIR%"
    )
)

if not exist "%BUILDDIR%\CMakeCache.txt" (
    echo [3/5] Configuring...
    cmake --preset %PRESET%
    if errorlevel 1 goto fail
) else (
    echo [3/5] Already configured - skipping. Pass "clean" to force a reconfigure.
)
echo.

rem ---------------------------------------------------------------------------
rem 4. Build
rem ---------------------------------------------------------------------------
echo [4/5] Building...
cmake --build --preset %PRESET% --parallel
if errorlevel 1 goto fail
echo.
echo       Note: "ninja: no work to do" is a success message. It means the
echo       outputs are already current, not that something went wrong.
echo.

rem ---------------------------------------------------------------------------
rem 5. Tests
rem ---------------------------------------------------------------------------
if "%RUN_TESTS%"=="1" (
    echo [5/5] Running tests...
    ctest --preset %PRESET% --output-on-failure
    if errorlevel 1 goto fail
) else (
    echo [5/5] Tests skipped.
)
echo.

rem ---------------------------------------------------------------------------
rem Report artefacts
rem ---------------------------------------------------------------------------
set "VST3=%BUILDDIR%\src\plugin\SampleBox_VST3_artefacts\%CONFIG%\VST3\Sample Box.vst3"
set "STANDALONE="
for /f "usebackq delims=" %%f in (`dir /s /b "%BUILDDIR%\src\app\*.exe" 2^>nul`) do set "STANDALONE=%%f"

echo ===========================================================================
echo  Build succeeded
echo ===========================================================================
if exist "%VST3%" (
    echo  VST3       : %VST3%
) else (
    echo  VST3       : [not found] expected at
    echo               %VST3%
)
if defined STANDALONE (
    echo  Standalone : !STANDALONE!
) else (
    echo  Standalone : [not found] under %BUILDDIR%\src\app
)
echo.

rem ---------------------------------------------------------------------------
rem Optional install of the plug-in for DAW testing
rem ---------------------------------------------------------------------------
if "%DO_INSTALL%"=="1" (
    set "VST3DEST=%CommonProgramFiles%\VST3"
    if not exist "%VST3%" (
        echo [ERROR] Cannot install: the VST3 bundle was not found.
        goto fail
    )
    echo Installing the plug-in to "!VST3DEST!"...
    echo A DAW must be fully closed for this to succeed, since it locks the file.
    robocopy "%VST3%" "!VST3DEST!\Sample Box.vst3" /MIR /NJH /NJS /NP /NDL >nul
    rem robocopy exit codes below 8 indicate success of some kind.
    if errorlevel 8 (
        echo [ERROR] Copy failed. Close your DAW, or re-run this script as
        echo         Administrator - "%CommonProgramFiles%\VST3" needs elevation.
        goto fail
    )
    echo Installed. Rescan your plug-in folders in your DAW to pick it up.
    echo.
)

popd
endlocal
exit /b 0

:fail
echo.
echo ===========================================================================
echo  BUILD FAILED
echo ===========================================================================
popd 2>nul
endlocal
exit /b 1
