@echo off
setlocal enabledelayedexpansion

:: Validate parameters
if "%~1"=="" (
    echo Error: Missing certificate file path.
    echo Usage: %~nx0 ^<cert_path^> ^<cert_thumbprint^> [^<reg_output_file^>]
    echo Example: %~nx0 cert.cer 4ce76... output.reg
    exit /b 1
)

if "%~2"=="" (
    echo Error: Missing certificate thumbprint.
    echo Usage: %~nx0 ^<cert_path^> ^<cert_thumbprint^> [^<reg_output_file^>]
    echo Example: %~nx0 cert.cer 4ce76... output.reg
    exit /b 1
)

:: Optional output file path
if "%~3"=="" (
    set "cert_reg_output=c:\temp\oem_sample_cert.reg"
) else (
    set "cert_reg_output=%~3"

    rem Validate .reg extension using delayed expansion
    setlocal enabledelayedexpansion
    set "reg_ext=!cert_reg_output:~-4!"
    if /i not "!reg_ext!"==".reg" (
        echo Error: The output file must have a .reg extension.
        echo Provided: %cert_reg_output%
        echo Usage: %~nx0 ^<cert_path^> ^<cert_thumbprint^> [^<reg_output_file.reg^>]
        endlocal
        exit /b 1
    )
    endlocal
)


set "cert_path=%~1"
set "cert_thumbprint=%~2"

call :ToLower "%cert_thumbprint%"
set "cert_thumbprint=%__ToLowerStr%"



set "cert_reg_temp=%TEMP%\oem_sample_cert_temp.reg"

:: Ensure the output directory exists
for %%D in ("%cert_reg_output%") do (
    if not exist "%%~dpD" (
        mkdir "%%~dpD"
    )
)

echo Cert file is %cert_path%

:: Check if certificate file exists
if exist "%cert_path%" (
    echo %cert_path% exists
) else (
    echo %cert_path% does not exist, please check your certificate path
    exit /b 1
)

:: Add certificate to TestSignRoot store
echo Adding OEM Test Certificate to certificate store...
certutil -addstore -f "TestSignRoot" "%cert_path%"
if errorlevel 1 (
    echo Failed to add certificate to store.
    exit /b 1
)

:: Export registry key
echo Exporting OEM Test Certificate registry...
reg export "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\SystemCertificates\TestSignRoot\Certificates\%cert_thumbprint%" "%cert_reg_output%" /y

if not exist "%cert_reg_output%" (
    echo ERROR: Failed to export registry key. Check if the thumbprint is correct and lowercase.
    exit /b 1
)

:: Replace 'software' with 'WinVOS-SOFTWARE'
echo Replacing 'software' with 'WinVOS-SOFTWARE'...
if exist "%cert_reg_temp%" del "%cert_reg_temp%" >nul 2>&1

for /f "delims=" %%A in ('type "%cert_reg_output%"') do (
    set "line=%%A"
    call set "line=%%line:software=WinVOS-SOFTWARE%%"
    >>"%cert_reg_temp%" echo(!line!
)

:: Replace original file with the modified one
del "%cert_reg_output%" >nul 2>&1
move /Y "%cert_reg_temp%" "%cert_reg_output%" >nul

if exist "%cert_reg_output%" (
    echo File oem_sample_cert.reg generated successfully at %cert_reg_output%
) else (
    echo Failed to generate registry file.
    exit /b 1
)

endlocal
exit /b 0

REM Takes a string as the first argument
REM Retruns the lower case version of the sting as __ToLowerStr
:ToLower
setlocal enabledelayedexpansion
set "_STRING=%~1"
set "_UCASE=ABCDEFGHIJKLMNOPQRSTUVWXYZ"
set "_LCASE=abcdefghijklmnopqrstuvwxyz"

for /l %%a in (0,1,25) do (
    call set "_FROM=%%_UCASE:~%%a,1%%"
    call set "_TO=%%_LCASE:~%%a,1%%"
    call set "_STRING=%%_STRING:!_FROM!=!_TO!%%"
)

endlocal & set "__ToLowerStr=%_STRING%"
exit /b 0
