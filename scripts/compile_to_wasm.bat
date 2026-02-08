@echo off
REM Compile C/C++ source to WebAssembly (.wasm) using WASI-SDK
REM Usage: compile_to_wasm.bat <input.c> <output.wasm> [additional_flags]

setlocal enabledelayedexpansion

if "%~1"=="" (
    echo Usage: %~nx0 ^<input.c^> ^<output.wasm^> [additional_flags]
    echo.
    echo Example:
    echo   %~nx0 example.c example.wasm
    echo   %~nx0 example.cpp example.wasm -O2
    echo   %~nx0 lib.c lib.wasm "-O2 -Wl,--export=myfunc"
    exit /b 1
)

if "%~2"=="" (
    echo ERROR: Output file not specified
    echo Usage: %~nx0 ^<input.c^> ^<output.wasm^> [additional_flags]
    exit /b 1
)

set "INPUT_FILE=%~1"
set "OUTPUT_FILE=%~2"
set "EXTRA_FLAGS=%~3 %~4 %~5 %~6 %~7 %~8 %~9"
set "WASI_SDK_DIR=..\third_party\wasi"

REM Check if input file exists
if not exist "%INPUT_FILE%" (
    echo ERROR: Input file not found: %INPUT_FILE%
    exit /b 1
)

REM Check if WASI-SDK is installed
if not exist "%WASI_SDK_DIR%\bin\clang.exe" (
    echo ERROR: WASI-SDK not found at %WASI_SDK_DIR%
    echo Please run: scripts\download_wasi_sdk.bat
    exit /b 1
)

REM Determine if it's C or C++ based on extension
set "EXT=%INPUT_FILE:~-4%"
set "COMPILER=clang.exe"
if /i "%EXT%"==".cpp" set "COMPILER=clang++.exe"
if /i "%EXT%"==".cxx" set "COMPILER=clang++.exe"
if /i "%EXT%"==".cc" set "COMPILER=clang++.exe"

echo ========================================
echo Compiling to WebAssembly
echo ========================================
echo Input:    %INPUT_FILE%
echo Output:   %OUTPUT_FILE%
echo Compiler: %COMPILER%
echo Flags:    %EXTRA_FLAGS%
echo ========================================
echo.

REM Create output directory if needed
for %%F in ("%OUTPUT_FILE%") do (
    if not "%%~dpF"=="" (
        if not exist "%%~dpF" mkdir "%%~dpF"
    )
)

REM Compile
"%WASI_SDK_DIR%\bin\%COMPILER%" ^
    -o "%OUTPUT_FILE%" ^
    "%INPUT_FILE%" ^
    %EXTRA_FLAGS%

if errorlevel 1 (
    echo.
    echo ========================================
    echo Compilation FAILED!
    echo ========================================
    exit /b 1
)

echo.
echo ========================================
echo Compilation successful!
echo ========================================
echo Output file: %OUTPUT_FILE%

REM Show file size
for %%A in ("%OUTPUT_FILE%") do (
    echo File size: %%~zA bytes
)

echo.
echo Next steps:
echo 1. Run with WAMR or WasmEdge runtime
echo 2. Or compile to AOT: scripts\compile_to_aot.bat %OUTPUT_FILE% output.aot
echo.

exit /b 0
