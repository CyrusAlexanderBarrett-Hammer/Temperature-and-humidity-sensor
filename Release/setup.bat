@echo off
setlocal enabledelayedexpansion

:: Define variables
set PYTHON_INSTALLER_URL=https://www.python.org/ftp/python/3.6.8/python-3.6.8.exe
set PYTHON_INSTALLER_PATH=%USERPROFILE%\Downloads\python-3.6.8.exe
set GET_PIP_URL=https://bootstrap.pypa.io/pip/3.6/get-pip.py
set GET_PIP_PATH=%TEMP%\get-pip.py
set PYTHON_PATH=%USERPROFILE%\AppData\Local\Programs\Python\Python36-32
set PIP_PATH=%PYTHON_PATH%\Scripts\pip.exe

:: set PYTHON_INSTALLER_URL=https://www.python.org/ftp/python/3.11.3/python-3.11.3-amd64.exe
:: set PYTHON_INSTALLER_PATH=%USERPROFILE%\Downloads\python-3.11.3.exe
:: set GET_PIP_URL=https://bootstrap.pypa.io/pip/get-pip.py
:: set GET_PIP_PATH=%TEMP%\get-pip.py
:: set PYTHON_PATH=%USERPROFILE%\AppData\Local\Programs\Python\Python311
:: set PIP_PATH=%PYTHON_PATH%\Scripts\pip.exe

:: Get the Windows version
for /f "tokens=4-5 delims=. " %%i in ('ver') do set USE_THIS_VERSION=%%i.%%j
:: Extract the major version
for /f "tokens=1 delims=." %%i in ("%USE_THIS_VERSION%") do set USE_THIS_MAJOR_VERSION=%%i

:: Download Python 3.6.8 x86 installer
::echo Downloading Python 3.6.8 installer...
::powershell.exe -Command "Invoke-WebRequest -Uri %PYTHON_INSTALLER_URL% -OutFile \"$env:PYTHON_INSTALLER_PATH\""

:: Use curl on Windows 10 and above
if %USE_THIS_MAJOR_VERSION% geq 10 (
    echo Downloading using curl...
    curl -o %PYTHON_INSTALLER_PATH% %PYTHON_INSTALLER_URL%
) else (
    echo Downloading using bitsadmin...
    bitsadmin /transfer myDownloadJob /download /priority normal %PYTHON_INSTALLER_URL% %CD%\%PYTHON_INSTALLER_PATH%
)

:: Check if the download was successful
if exist %PYTHON_INSTALLER_PATH% (
    echo Download successful.
) else (
    echo Download failed. Python needs to be downloaded manually. Go to PYTHON_INSTALLER_URL=https://www.python.org/ftp/python/3.6.8/python-3.6.8.exe and press any key when done.
    pause
)

:: Install Python 3.6.8
echo Installing Python 3.6.8...
echo Installer path is %PYTHON_INSTALLER_PATH%
start /wait "" %PYTHON_INSTALLER_PATH% /quiet InstallAllUsers=0 PrependPath=0 

if exist %PYTHON_PATH% (
    echo Python installation successful.
) else (
    echo Python installation failed.
    pause
)

pause

:: Download get-pip.py
::echo Downloading get-pip.py...
::powershell.exe -Command "Invoke-WebRequest -Uri %GET_PIP_URL% -OutFile \"$env:GET_PIP_PATH\""

if %USE_THIS_MAJOR_VERSION% geq 10 (
    echo Downloading get-pip.py using curl...
    curl -o %GET_PIP_PATH% %GET_PIP_URL%
) else (
    echo Downloading get-pip.py using bitsadmin...
    bitsadmin /transfer myDownloadJob /download /priority normal %GET_PIP_URL% %CD%\%GET_PIP_PATH%
)

:: Install pip
echo Installing pip...
%PYTHON_PATH%\python.exe %GET_PIP_PATH%

pause

:: Checking if PATH length limit is enabled
:: Initialize variable to hold the registry value
set "LongPathsEnabled="

:: Query the registry for LongPathsEnabled
for /f "tokens=2,*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\FileSystem" /v "LongPathsEnabled" 2^>nul') do (
    set "LongPathsEnabled=%%b"
)

set "UserPath="

:: Query the registry for the current System PATH value
for /f "tokens=2,* skip=2" %%a in ('reg query "HKCU\Environment" /v "PATH"') do (
    set "UserPath=%%b"
)

set "UserPath=%UserPath%;%PYTHON_PATH%"
set "UserPath=%UserPath%;%PYTHON_PATH%\Scripts"

:: Check for PATH length
echo %UserPath%
if defined UserPath if "!LongPathsEnabled!"=="0x1" if "!UserPath:~0,1024!" neq "!UserPath!" (
    echo Error: New PATH is above PATH length limit.
    echo PATH is too long, so the installation will not work. Ask IT to disable PATH limit, then rerun the BATCH script.
    pause
    exit
)

setx PATH "%UserPath%"

:: Call PowerShell command to remove duplicates from PATH
for /f "delims=" %%i in ('powershell -command "[String]::Join(';', (([System.Environment]::GetEnvironmentVariable('PATH', [System.EnvironmentVariableTarget]::User) -split ';') | Sort-Object -Unique))"') do set UserPath=%%i

:: If everything is ok, set the new PATH
echo PATH set
pause

:: Install Python libraries
echo Installing required Python libraries...
if exist %PIP_PATH% echo pip exists
::%PIP_PATH% install pyserial
::%PIP_PATH% install serial
::%PIP_PATH% install tk
%PYTHON_PATH%\python.exe -m pip install serial
%PYTHON_PATH%\python.exe -m pip install pyserial
%PYTHON_PATH%\python.exe -m pip install tk
echo Libraries installed
pause

:: Clean up
echo Cleaning up...
del %PYTHON_INSTALLER_PATH%
del %GET_PIP_PATH%

echo Installation completed.
endlocal

pause