@echo off
rem Build and run the desktop tests of the HP virtual machines and TOTP (Visual Studio C++ tools required)
setlocal
set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set VSDIR=%%i
if not defined VSDIR (echo Visual Studio with the C++ tools not found & exit /b 1)
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>nul
where cl >nul 2>nul || (echo cl not found, vcvars64.bat of %VSDIR% failed & exit /b 1)

cd /d "%~dp0"
if not exist build mkdir build
set CL_OPTS=/nologo /EHsc /std:c++17 /W3 /wd4244 /wd4305 /wd4309 /I "..\..\libraries\TinyRPNCalculator\src" /Fo"build\\"
cl %CL_OPTS% /Fe"build\hp35test.exe" src\hp35test.cpp || exit /b 1
cl %CL_OPTS% /Fe"build\hp21test.exe" src\hp21test.cpp src\hp21rom_x11calc.cpp || exit /b 1
cl %CL_OPTS% /Fe"build\totptest.exe" src\totptest.cpp || exit /b 1

rem exit code = total number of failures, a crashed test counts as one
set FAILS=0
for %%t in (hp35test hp21test totptest) do call :run %%t
exit /b %FAILS%

:run
build\%1.exe
set RC=%ERRORLEVEL%
if %RC% lss 0 set RC=1
set /a FAILS+=RC
exit /b 0
