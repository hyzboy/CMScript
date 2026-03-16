@echo off
REM Download WASI-SDK for Windows
REM This script downloads the latest WASI-SDK release and extracts it to third_party/wasi

setlocal enabledelayedexpansion

echo ========================================
echo WASI-SDK Download Script for Windows
echo ========================================
echo.

REM Configuration
set "WASI_VERSION=22"
set "WASI_FULL_VERSION=wasi-sdk-22.0"
set "DOWNLOAD_URL=https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-22/%WASI_FULL_VERSION%-mingw.tar.gz"
set "TARGET_DIR=..\third_party\wasi"
set "TEMP_FILE=wasi-sdk-temp.tar.gz"

echo Target directory: %TARGET_DIR%
echo Download URL: %DOWNLOAD_URL%
echo.

REM Create target directory
if not exist "%TARGET_DIR%" (
    echo Creating directory: %TARGET_DIR%
    mkdir "%TARGET_DIR%"
)

REM Check if already downloaded
if exist "%TARGET_DIR%\bin\clang.exe" (
    echo.
    echo WASI-SDK appears to be already installed in %TARGET_DIR%
    echo.
    choice /C YN /M "Do you want to re-download and overwrite"
    if errorlevel 2 (
        echo Keeping existing installation.
        goto :verify
    )
    echo.
    echo Removing existing installation...
    rd /s /q "%TARGET_DIR%"
    mkdir "%TARGET_DIR%"
)

REM Download WASI-SDK
echo.
echo Downloading WASI-SDK %WASI_VERSION%...
echo This may take a few minutes...
echo.

REM Try PowerShell download first
powershell -Command "try { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $ProgressPreference = 'SilentlyContinue'; Invoke-WebRequest -Uri '%DOWNLOAD_URL%' -OutFile '%TEMP_FILE%' -UseBasicParsing } catch { exit 1 }"

if errorlevel 1 (
    echo.
    echo PowerShell download failed. Trying curl...
    curl -L -o "%TEMP_FILE%" "%DOWNLOAD_URL%"
    if errorlevel 1 (
        echo.
        echo ERROR: Failed to download WASI-SDK
        echo Please download manually from:
        echo https://github.com/WebAssembly/wasi-sdk/releases
        goto :error
    )
)

echo Download completed!
echo.

REM Extract the archive
echo Extracting WASI-SDK...
echo.

REM Check if tar is available (Windows 10 1803+)
where tar >nul 2>nul
if %errorlevel% equ 0 (
    echo Using Windows built-in tar...
    tar -xzf "%TEMP_FILE%" -C "%TARGET_DIR%" --strip-components=1
) else (
    echo.
    echo ERROR: tar command not found.
    echo Please extract %TEMP_FILE% manually to %TARGET_DIR%
    echo Or install 7-Zip and try again.
    goto :error
)

if errorlevel 1 (
    echo.
    echo ERROR: Failed to extract archive
    echo The file may be corrupted. Please try again.
    goto :error
)

REM Clean up
echo.
echo Cleaning up temporary files...
del /f /q "%TEMP_FILE%"

:verify
REM Verify installation
echo.
echo Verifying installation...
echo.

if exist "%TARGET_DIR%\bin\clang.exe" (
    echo Success! WASI-SDK installed successfully.
    echo.
    echo Verifying clang version:
    "%TARGET_DIR%\bin\clang.exe" --version
    echo.
    echo Installation complete!
    echo.
    echo WASI-SDK is now available at: %TARGET_DIR%
    echo.
    echo Next steps:
    echo 1. Compile C/C++ to .wasm: scripts\compile_to_wasm.bat
    echo 2. Read the guide: docs\WASI_SDK_GUIDE.md
    goto :end
) else (
    echo.
    echo ERROR: Installation verification failed.
    echo clang.exe not found in %TARGET_DIR%\bin\
    goto :error
)

:error
echo.
echo ========================================
echo Installation failed!
echo ========================================
exit /b 1

:end
echo.
echo ========================================
echo Installation successful!
echo ========================================
exit /b 0
