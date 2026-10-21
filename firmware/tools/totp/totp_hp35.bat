@echo off
rem TOTP keys of the hp35 sketch:
rem 1. totp.h with the test keys of data\totp.txt, it is built into the firmware
rem 2. build\hp35.totp.eep - EEPROM image with your keys from data\totp.local.txt
rem    (ignored by git), flash it instead of build\hp35.ino.eep
rem The keys area size must match TOTP_AREA_SIZE below, it fills all free EEPROM.
rem Started from Explorer, the window waits for a key at the end, so that the
rem messages and the current codes can be read.
setlocal
set TOTP_AREA_SIZE=126
set SKETCH=%~dp0..\..\sketches\hp35
set TOTP=%~dp0totp.exe
set RESULT=0

"%TOTP%" header "%SKETCH%\data\totp.txt" %TOTP_AREA_SIZE% "%SKETCH%\totp.h" || goto failed
echo.

if not exist "%SKETCH%\data\totp.local.txt" (
	echo No data\totp.local.txt with your keys, EEPROM image is not created.
	goto done
)
if not exist "%SKETCH%\build\hp35.ino.eep" (
	echo No build\hp35.ino.eep, export compiled binary in Arduino IDE first.
	goto failed
)
"%TOTP%" eep "%SKETCH%\data\totp.local.txt" "%SKETCH%\build\hp35.ino.eep" "%SKETCH%\build\hp35.totp.eep" || goto failed
goto done

:failed
set RESULT=1

:done
rem the command line of cmd has the path of this file only when Explorer started it
echo %cmdcmdline% | find /i "%~f0" >nul && pause
exit /b %RESULT%
