@echo off
setlocal

set "LIB_NAME=TinyRPNCalculaor"

net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Error: Administrator privileges required. Run as administrator.
    pause
    exit /b 1
)

set "TARGET=%USERPROFILE%\Documents\Arduino\libraries\%LIB_NAME%"
set "SOURCE=%~dp0%LIB_NAME%"

if exist "%TARGET%" rmdir "%TARGET%" 2>nul
mklink /D "%TARGET%" "%SOURCE%"

pause