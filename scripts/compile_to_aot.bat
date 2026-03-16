@echo off
REM Compile WebAssembly (.wasm) to AOT format (.aot) using WAMR compiler
REM Usage: compile_to_aot.bat <input.wasm> <output.aot> [additional_flags]

setlocal enabledelayedexpansion

if "%~1"=="" (
    echo Usage: %~nx0 ^<input.wasm^> ^<output.aot^> [additional_flags]
    echo.
    echo Example:
    echo   %~nx0 example.wasm example.aot
    echo   %~nx0 example.wasm example.aot --opt-level=3
    echo   %~nx0 example.wasm example.aot "--target=x86_64 --opt-level=3"
    exit /b 1
)

if "%~2"=="" (
    echo ERROR: Output file not specified
    echo Usage: %~nx0 ^<input.wasm^> ^<output.aot^> [additional_flags]
    exit /b 1
)

set "INPUT_FILE=%~1"
set "OUTPUT_FILE=%~2"
set "EXTRA_FLAGS=%~3 %~4 %~5 %~6 %~7 %~8 %~9"
set "WAMR_DIR=..\third_party\wasm-micro-runtime"
set "WAMRC_EXE=%WAMR_DIR%\wamr-compiler\build\wamrc.exe"

REM Check if input file exists
if not exist "%INPUT_FILE%" (
    echo ERROR: Input file not found: %INPUT_FILE%
    exit /b 1
)

REM Check if wamrc exists
if not exist "%WAMRC_EXE%" (
    echo ERROR: wamrc compiler not found at %WAMRC_EXE%
    echo.
    echo Please build WAMR AOT compiler first:
    echo   1. Initialize submodules: git submodule update --init --recursive
    echo   2. cd %WAMR_DIR%\wamr-compiler
    echo   3. mkdir build ^&^& cd build
    echo   4. cmake ..
    echo   5. cmake --build . --config Release
    echo.
    exit /b 1
)

echo ========================================
echo Compiling to AOT format
echo ========================================
echo Input:  %INPUT_FILE%
echo Output: %OUTPUT_FILE%
echo Flags:  %EXTRA_FLAGS%
echo ========================================
echo.

REM Create output directory if needed
for %%F in ("%OUTPUT_FILE%") do (
    if not "%%~dpF"=="" (
        if not exist "%%~dpF" mkdir "%%~dpF"
    )
)

REM Compile
"%WAMRC_EXE%" ^
    -o "%OUTPUT_FILE%" ^
    %EXTRA_FLAGS% ^
    "%INPUT_FILE%"

if errorlevel 1 (
    echo.
    echo ========================================
    echo AOT Compilation FAILED!
    echo ========================================
    exit /b 1
)

echo.
echo ========================================
echo AOT Compilation successful!
echo ========================================
echo Output file: %OUTPUT_FILE%

REM Show file size comparison
echo.
echo File size comparison:
for %%A in ("%INPUT_FILE%") do (
    echo   .wasm: %%~zA bytes
)
for %%A in ("%OUTPUT_FILE%") do (
    echo   .aot:  %%~zA bytes
)

echo.
echo Next steps:
echo 1. Load AOT module in WAMR runtime
echo 2. Enjoy faster execution!
echo.

exit /b 0
