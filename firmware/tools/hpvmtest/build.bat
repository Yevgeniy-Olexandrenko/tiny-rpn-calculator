@echo off
rem Build and run the desktop tests of the HP virtual machines (Visual Studio C++ tools required)
setlocal
set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -property installationPath`) do set VSDIR=%%i
if not defined VSDIR (echo Visual Studio not found & exit /b 1)
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>nul

cd /d "%~dp0"
if not exist build mkdir build
cl /nologo /EHsc /std:c++17 /W3 /wd4244 /wd4305 /wd4309 /I "..\..\libraries\TinyRPNCalculator\src" ^
	/Fo"build\\" /Fe"build\hp21test.exe" src\hp21test.cpp src\hp21rom_x11calc.cpp || exit /b 1
build\hp21test.exe
