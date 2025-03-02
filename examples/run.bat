@echo off && setlocal EnableDelayedExpansion && cd /d "%~dp0" && call "%~dp0..\script\intro.bat"

set "Options=/std:c++20 /O2 /EHsc /I ..\include"

echo %MSVC% %Options% example1.c
call %MSVC% %Options% example1.c   || goto :ERROR
                    .\example1.exe || goto :ERROR

echo %MSVC% %Options% example2.c
call %MSVC% %Options% example2.c   || goto :ERROR
                    .\example2.exe || goto :ERROR

:OK
%Exit_OK%

:ERROR
%Exit_NG%
