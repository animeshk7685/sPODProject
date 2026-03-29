@echo off

set projectDir=%1
set outputDir=%2
set projectName=%3
set /A type=%4

echo start post_build.bat: %projectDir% %outputDir% %projectName% %type%


IF %type%==1 CALL :copyOtaDep

IF %type%==2 CALL :copyOtaDep
IF %type%==2 CALL :copyCyacd

IF %type%==3 CALL :copyHex
IF %type%==3 CALL :copyCyacd

	
echo post_build.bat finished!
EXIT /B %ERRORLEVEL% 


:copyOtaDep
set otaDir=%projectDir%\OtaDependencies\
rem make sure directory actually exists...
if not exist "%otaDir%" (
	mkdir %otaDir% >NUL
	echo needs %otaDir%: creating...
)

rem needed for ota positioning
set elfFile=%outputDir%\%projectName%.elf
set hexFile=%outputDir%\%projectName%.hex
set csiFile=%outputDir%\cycodeshareimport.ld
set bleFile=%projectDir%\Generated_Source\PSoC4\CyBle.cycsa

if exist "%elfFile%" (
	copy %elfFile% %otaDir% >NUL
	echo Copied: %elfFile% to %otaDir%
)
if exist "%hexFile%" (
	copy %hexFile% %otaDir% >NUL
	echo Copied: %hexFile% to %otaDir%
)
if exist "%csiFile%" (
	copy %csiFile% %otaDir% >NUL
	echo Copied: %csiFile% to %otaDir%
)
if exist "%bleFile%" (
	copy %bleFile% %otaDir% >NUL
	echo Copied: %bleFile% to %otaDir%
)
EXIT /B 0

:copyCyacd
set newDir=%projectDir%\..\_BuildOutput\
rem make sure directory actually exists...
if not exist "%newDir%" (
	mkdir %newDir% >NUL
	echo needs %newDir%: creating...
)

rem main device ota file, needed for ota programming
set otaFile=%outputDir%\%projectName%.cyacd
if exist "%otaFile%" (
	copy %otaFile% %newDir% >NUL
	echo Copied: %otaFile% to %newDir%
)
EXIT /B 0

:copyHex
set newDir=%projectDir%\..\_BuildOutput\
rem make sure directory actually exists...
if not exist "%newDir%" (
	mkdir %newDir% >NUL
	echo needs %newDir%: creating...
)

rem main device hex, needed for programming
set hexFile=%outputDir%\%projectName%.hex
if exist "%hexFile%" (
	copy %hexFile% %newDir% >NUL
	echo Copied: %hexFile% to %newDir%
)
EXIT /B 0

rem ${ProjectDir}/../Post_Build.bat ${ProjectDir} ${OutputDir} ${ProjectShortName} 1-3
