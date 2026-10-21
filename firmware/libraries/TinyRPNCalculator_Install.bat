@echo off
setlocal

set "LIB_NAME=TinyRPNCalculator"

net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Error: Administrator privileges required. Run as administrator.
    pause
    exit /b 1
)

rem 1. Link the library into the libraries folder of the Arduino sketchbook
set "TARGET=%USERPROFILE%\Documents\Arduino\libraries\%LIB_NAME%"
set "SOURCE=%~dp0%LIB_NAME%"

if exist "%TARGET%" rmdir "%TARGET%" 2>nul
mklink /D "%TARGET%" "%SOURCE%"
echo.

rem 2. Export hooks: "Sketch > Export compiled binary" of the Arduino IDE puts
rem    only the .hex file next to the sketch, the .eep file stays in a temporary
rem    folder. A platform.local.txt next to platform.txt of the core adds hooks
rem    that copy both files to the build folder of the sketch. It is made for
rem    every installed version of ATTinyCore and TinyCore, run this file again
rem    after a core update.
set "PACKAGES=%LOCALAPPDATA%\Arduino15\packages"
set "MARKER=# TinyRPNCalculator export hooks"
set "FOUND=0"
for /d %%D in ("%PACKAGES%\ATTinyCore\hardware\avr\*" "%PACKAGES%\TinyCore\hardware\avr\*") do call :hooks "%%~D"
if "%FOUND%"=="0" echo No ATTinyCore or TinyCore in %PACKAGES%, export hooks are not added.

pause
exit /b 0

rem Writes platform.local.txt with the export hooks into the core folder %1,
rem a platform.local.txt with other settings of the user is not changed
:hooks
set "FOUND=1"
set "FILE=%~1\platform.local.txt"
if exist "%FILE%" findstr /b /c:"%MARKER%" "%FILE%" >nul || (
    echo Skipped %FILE%: it has other settings, add the export hooks to it manually.
    exit /b 0
)
> "%FILE%" (
    echo %MARKER%: Export compiled binary also copies .hex and .eep to sketch\build
    echo recipe.hooks.savehex.postsavehex.1.pattern.windows=cmd /C if not exist "{sketch_path}\build" mkdir "{sketch_path}\build"
    echo recipe.hooks.savehex.postsavehex.2.pattern.windows=cmd /C copy /Y "{build.path}\{build.project_name}.hex" "{sketch_path}\build" ^>nul
    echo recipe.hooks.savehex.postsavehex.3.pattern.windows=cmd /C copy /Y "{build.path}\{build.project_name}.eep" "{sketch_path}\build" ^>nul
)
echo Export hooks: %FILE%
exit /b 0
