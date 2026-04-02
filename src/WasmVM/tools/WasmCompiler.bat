:found_clang
@echo off
REM WAMR script compilation helper - Windows
REM Output example: game_script.wasm (wasm32-unknown-unknown)

setlocal

REM --- Parse arguments ---
set "INPUT="
set "OUTPUT="

if "%~1"=="" goto usage

:parse_args
if "%~1"=="" goto args_done
if "%~1"=="-h" goto usage
if "%~1"=="--help" goto usage
if "%~1"=="-o" (
  shift
  if "%~1"=="" goto usage
  set "OUTPUT=%~1"
  shift
  goto parse_args
)
if "%~1"=="-O" (
  shift
  if "%~1"=="" goto usage
  set "OUTPUT=%~1"
  shift
  goto parse_args
)
if not defined INPUT (
  set "INPUT=%~1"
  shift
  goto parse_args
)
if not defined OUTPUT (
  set "OUTPUT=%~1"
  shift
  goto parse_args
)
shift
goto parse_args

:args_done

if not defined INPUT goto usage

REM Default output filename: <input-basename>.wasm
if not defined OUTPUT (
  for %%F in ("%INPUT%") do set "OUTPUT=%%~nF.wasm"
)

REM --- Locate clang ---
if "%CLANG%"=="" set "CLANG=C:\clang\bin\clang.exe"

if not exist "%CLANG%" (
    for /f "delims=" %%i in ('where clang 2^>nul') do (
        set "CLANG=%%i"
        goto found_clang
    )
)

:found_clang
if not exist "%CLANG%" (
    echo Error: clang not found.
    echo You can:
    echo   1) set CLANG=C:\path\to\clang.exe
    echo   2) add clang to your PATH
    echo   3) place clang at .\clang\bin\clang.exe
    exit /b 1
)

echo Using clang: %CLANG%
echo Compiling %INPUT% -> %OUTPUT% ...

"%CLANG%" ^
  --target=wasm32 ^
  -O2 ^
  -nostdlib ^
  -Wl,--no-entry ^
  -Wl,--allow-undefined ^
  -Wl,--export=script_init ^
  -Wl,--export=script_update ^
  -Wl,--export=calculate ^
  -Wl,--export-table ^
  -Wl,--strip-all ^
  -o "%OUTPUT%" "%INPUT%"

if %ERRORLEVEL% neq 0 (
  echo.
  echo Error: build failed.
  exit /b 1
)

echo.
echo Compiled successfully: %OUTPUT%
exit /b 0

:usage
echo WAMR script compiler - usage
echo.
echo Usage: %~nx0 input.c [-o output.wasm]
echo.
echo Examples:
echo   %~nx0 game_script.c -o game_script.wasm
echo   %~nx0 game_script.c game_script.wasm
exit /b 1
