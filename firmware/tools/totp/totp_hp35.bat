@echo off
rem TOTP keys of the hp35 sketch:
rem 1. totp.h with the test keys of data\totp.txt, it is built into the firmware
rem 2. build\hp35.totp.eep - EEPROM image with your keys from data\totp.local.txt
rem    (ignored by git), flash it instead of build\hp35.ino.eep
rem The keys area size must match TOTP_AREA_SIZE below, it fills all free EEPROM.
setlocal
set TOTP_AREA_SIZE=126
set SKETCH=%~dp0..\..\sketches\hp35
set TOTP=%~dp0totp.exe

"%TOTP%" header "%SKETCH%\data\totp.txt" %TOTP_AREA_SIZE% "%SKETCH%\totp.h" || exit /b 1
echo.

if not exist "%SKETCH%\data\totp.local.txt" (
	echo No data\totp.local.txt with your keys, EEPROM image is not created.
	exit /b 0
)
if not exist "%SKETCH%\build\hp35.ino.eep" (
	echo No build\hp35.ino.eep, export compiled binary in Arduino IDE first.
	exit /b 1
)
"%TOTP%" eep "%SKETCH%\data\totp.local.txt" "%SKETCH%\build\hp35.ino.eep" "%SKETCH%\build\hp35.totp.eep" || exit /b 1
