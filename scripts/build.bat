@echo off
setlocal EnableDelayedExpansion

rem ===========================================================================
rem Sample Box - one-step Windows build (x64 only).
rem
rem Run this from any shell, or just double-click it. It imports the Visual
rem Studio 2022 x64 build environment itself.
rem
rem Changes in this version:
rem   * Refuses to reuse a Visual Studio environment that is not x64. An x86
rem     Developer prompt produces a 32-bit VST3 that 64-bit hosts (Ableton)
rem     cannot load. A non-x64 environment is replaced by the x64 one.
rem   * Verifies the compiler really targets x64 before building.
rem   * Finds the repository root robustly and checks it, instead of assuming
rem     the script is in scripts\ and silently using the wrong folder.
rem   * Verifies the finished VST3 bundle is x86_64-win, not x86-win.
rem
rem Usage:
rem   scripts\build.bat                  Debug build, then run tests
rem   scripts\build.bat release          Release build, then run tests
rem   scripts\build.bat notest           Debug build, skip tests
rem   scripts\build.bat clean            Wipe the build dir, then rebuild
rem   scripts\build.bat release install  Release build, then copy the VST3
rem                                      into the system VST3 folder
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
if /i "%~1"=="debug" ( set "PRESET=windows-debug" & set "CONFIG=Debug" & shift & goto parse_args )
if /i "%~1"=="release" ( set "PRESET=windows-release" & set "CONFIG=Release" & shift & goto parse_args )
if /i "%~1"=="notest" ( set "RUN_TESTS=0" & shift & goto parse_args )
if /i "%~1"=="clean" ( set "DO_CLEAN=1" & shift & goto parse_args )
if /i "%~1"=="install" ( set "DO_INSTALL=1" & shift & goto parse_args )
echo [ERROR] Unrecognised argument: %~1
echo Valid arguments: debug release notest clean install
goto fail
:args_done

rem ---------------------------------------------------------------------------
rem Repository root: normally the parent of this script's folder (scripts\).
rem If this file was copied to the repo root instead, use its own folder.
rem ---------------------------------------------------------------------------
set "ROOT=%~dp0.."
if not exist "%ROOT%\CMakePresets.json" if exist "%~dp0CMakePresets.json" set "ROOT=%~dp0"
pushd "%ROOT%" || goto fail
set "ROOT=%CD%"

if not exist "%ROOT%\CMakePresets.json" (
echo.
echo [ERROR] This does not look like the Sample Box repository root:
echo         %ROOT%
echo         CMakePresets.json was not found there.
echo.
echo Keep this script at ^<repo^>\scripts\build.bat, or run it from a
echo checkout that contains CMakePresets.json.
goto fail
)

echo ===========================================================================
echo Sample Box build
echo Repository : %ROOT%
echo Preset     : %PRESET% ^(%CONFIG%^)
echo ===========================================================================
echo.

rem ---------------------------------------------------------------------------
rem 1. Visual Studio x64 environment
rem ---------------------------------------------------------------------------
set "TGT_ARCH=%VSCMD_ARG_TGT_ARCH%"
if not defined TGT_ARCH set "TGT_ARCH=%Platform%"

if /i "%TGT_ARCH%"=="x64" if defined VSCMD_VER goto vs_reuse
if /i "%TGT_ARCH%"=="x64" if defined VCINSTALLDIR goto vs_reuse

if defined TGT_ARCH (
echo [1/5] Current environment targets "%TGT_ARCH%", not x64. Importing the x64 environment instead.
) else (
echo [1/5] Importing the Visual Studio x64 build environment...
)
goto vs_import

:vs_reuse
echo [1/5] Already inside an x64 Visual Studio developer environment - reusing it.
goto vs_checked

:vs_import
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
echo.
echo [ERROR] Could not find vswhere.exe at:
echo         %VSWHERE%
echo.
echo That file ships with every Visual Studio 2022 install, so this
echo usually means Visual Studio 2022 or the Build Tools are not
echo installed. Install "Desktop development with C++" plus a
echo Windows 10 or 11 SDK, then run this script again.
goto fail
)

set "VSPATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do set "VSPATH=%%i"

if not defined VSPATH (
echo.
echo [ERROR] Visual Studio is installed, but no instance carries the C++
echo         toolchain ^(Microsoft.VisualStudio.Component.VC.Tools.x86.x64^).
echo.
echo Open the Visual Studio Installer, choose Modify, and enable
echo "Desktop development with C++" together with a Windows 10 or
echo 11 SDK.
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
echo Using: %VSPATH%

:vs_checked
rem Final proof: the compiler on PATH must report x64.
where cl >nul 2>&1 || (
echo [ERROR] cl.exe was not found on PATH after setting up the environment.
goto fail
)
cl 2>&1 | find "x64" >nul
if errorlevel 1 (
echo.
echo [ERROR] The C++ compiler on PATH does not target x64.
echo         A 32-bit build will not load in 64-bit hosts such as Ableton.
echo         Open "x64 Native Tools Command Prompt for VS 2022" and retry.
goto fail
)
echo Compiler targets x64 - OK.
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
echo JUCE is a local dependency and is deliberately not committed.
echo Install the validated revision from the repository root:
echo.
echo   git clone https://github.com/juce-framework/JUCE.git external\JUCE
echo   git -C external\JUCE checkout 7aae7d8e8deb8413bb01633d2795ef9974a181c5
echo.
echo See BUILDING.md for the full explanation.
goto fail
)

set "JUCEREV="
for /f "usebackq tokens=*" %%h in (`git -C "%ROOT%\external\JUCE" rev-parse HEAD 2^>nul`) do set "JUCEREV=%%h"
if defined JUCEREV (
echo JUCE revision: !JUCEREV!
if /i not "!JUCEREV!"=="7aae7d8e8deb8413bb01633d2795ef9974a181c5" (
echo [WARNING] That is not the revision this project was validated
echo           against ^(7aae7d8e8deb8413bb01633d2795ef9974a181c5^).
echo           The build may still work, but if it fails in JUCE
echo           itself rather than in src\, suspect this first.
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
echo Note: "ninja: no work to do" is a success message. It means the
echo outputs are already current, not that something went wrong.
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
rem Report and verify artefacts
rem ---------------------------------------------------------------------------
set "VST3=%BUILDDIR%\src\plugin\SampleBox_VST3_artefacts\%CONFIG%\VST3\Sample Box.vst3"
set "STANDALONE="
for /f "usebackq delims=" %%f in (`dir /s /b "%BUILDDIR%\src\app\*.exe" 2^>nul`) do set "STANDALONE=%%f"

echo ===========================================================================
echo Build succeeded
echo ===========================================================================
if exist "%VST3%" (
echo VST3       : %VST3%
) else (
echo VST3       : [not found] expected at
echo              %VST3%
)
if defined STANDALONE (
echo Standalone : !STANDALONE!
) else (
echo Standalone : [not found] under %BUILDDIR%\src\app
)

if exist "%VST3%\Contents\x86_64-win" (
echo VST3 arch  : x86_64-win - OK
) else if exist "%VST3%\Contents\x86-win" (
echo.
echo [ERROR] The VST3 bundle is 32-bit ^(Contents\x86-win^).
echo         It will not load in 64-bit hosts. Do a clean x64 rebuild:
echo         scripts\build.bat clean %CONFIG%
goto fail
) else if exist "%VST3%" (
echo [WARNING] Could not find Contents\x86_64-win in the VST3 bundle.
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
echo BUILD FAILED
echo ===========================================================================
popd 2>nul
endlocal
exit /b 1
