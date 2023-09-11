::@echo off
setlocal

:: Define variables
set PYTHON_INSTALLER_URL=https://www.python.org/ftp/python/3.6.8/python-3.6.8.exe
set PYTHON_INSTALLER_PATH=%USERPROFILE%\Downloads\python-3.6.8.exe
set GET_PIP_URL=https://bootstrap.pypa.io/pip/3.6/get-pip.py
set GET_PIP_PATH=%TEMP%\get-pip.py
set PYTHON_PATH=%USERPROFILE%\AppData\Local\Programs\Python\Python36-32
set PIP_PATH=%PYTHON_PATH%\Scripts\pip.exe

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

pause

:: Install Python 3.6.8
echo Installing Python 3.6.8...
echo Installer path is %PYTHON_INSTALLER_PATH%
echo Installation path is %PYTHON_PATH%
start /wait "" %PYTHON_INSTALLER_PATH% /quiet InstallAllUsers=0 PrependPath=1 TargetDir=%PYTHON_PATH%
echo Installation error level: %ERRORLEVEL%

if exist %PYTHON_INSTALLER_PATH% (echo Thank god)

pause

:: Download get-pip.py
::echo Downloading get-pip.py...
::powershell.exe -Command "Invoke-WebRequest -Uri %GET_PIP_URL% -OutFile \"$env:GET_PIP_PATH\""

if %USE_THIS_MAJOR_VERSION% geq 10 (
    echo Downloading get-pip.py using curl...
    pause
    curl -o %GET_PIP_PATH% %GET_PIP_URL%
) else (
    pause
    echo Downloading get-pip.py using bitsadmin...
    bitsadmin /transfer myDownloadJob /download /priority normal %GET_PIP_URL% %CD%\%GET_PIP_PATH%
)

pause

:: Install pip
echo Installing pip...
%PYTHON_PATH%\python.exe %GET_PIP_PATH%

:: Add Python and pip to PATH
echo Possible crash coming up? Setting path next.
pause
set NEW_PATH=%PATH%;%PYTHON_PATH%\Scripts
echo Path set...
pause
if defined NEW_PATH if "%NEW_PATH:~0,1024%" neq "%NEW_PATH%" (
    echo Error: New PATH exceeds the 1024 character limit imposed by setx.
    PATH is over 1028 characters long, so the installation will not work. Ask IT to disable PATH limit, then try again. Press any key to continue.
    pause
    exit
)
setx PATH "%NEW_PATH%"

:: Install Python libraries
echo Installing required Python libraries...
%PIP_PATH% install pyserial
%PIP_PATH% install serial
%PIP_PATH% install tk
pause

:: Clean up
echo Cleaning up...
del %PYTHON_INSTALLER_PATH%
del %GET_PIP_PATH%

pause

echo Installation completed.
endlocal

pause