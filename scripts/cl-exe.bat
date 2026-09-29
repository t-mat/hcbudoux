@echo off
setlocal enabledelayedexpansion
set /a errorno=1

rem Set Visual C++ environment for Windows, desktop, x64.
rem
rem https://github.com/Microsoft/vswhere
rem https://github.com/microsoft/vswhere/wiki/Find-VC#batch

set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" (
  echo Failed to find "vswhere.exe".  Please install the latest version of Visual Studio.
  goto :ERROR
)

set "InstallDir="
for /f "usebackq tokens=*" %%i in (
  `"%vswhere%" -latest ^
               -products * ^
               -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
               -property installationPath`
) do (
  set "InstallDir=%%i"
)
if "%InstallDir%" == "" (
  echo Failed to find Visual C++.  Please install the latest version of Visual C++.
  goto :ERROR
)

set "vcvars64bat=%InstallDir%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%vcvars64bat%" (
  echo Failed to find "vcvars64.bat".  Please install the latest version of Visual C++.
  goto :ERROR
)

call "%vcvars64bat%" >nul || goto :ERROR

cl.exe %* || goto :ERROR

set /a errorno=0
goto :END

:ERROR

:END
exit /B %errorno%
