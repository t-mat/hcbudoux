@echo off && setlocal EnableDelayedExpansion && cd /d "%~dp0" && call "%~dp0..\script\intro.bat"

set "Options=/std:c++20 /O2 /EHsc /I ..\include /I ..\third_party\json.h"

echo %MSVC% %Options% codegen.cpp
call %MSVC% %Options% codegen.cpp || goto :ERROR
                    .\codegen.exe || goto :ERROR

:OK
%Exit_OK%

:ERROR
%Exit_NG%
