@echo off && setlocal EnableDelayedExpansion && cd /d "%~dp0" && call "%~dp0..\script\intro.bat"

set "COptions=/std:c11 /O2 /I ..\include"
set "CxxOptions=/std:c++14 /O2 /EHsc /I ..\include"

echo %MSVC% %COptions% test1.c
call %MSVC% %COptions% test1.c   || goto :ERROR
                    .\test1.exe || goto :ERROR

echo %MSVC% %CxxOptions% test2.cpp
call %MSVC% %CxxOptions% test2.cpp || goto :ERROR
                    .\test2.exe || goto :ERROR

:OK
%Exit_OK%

:ERROR
%Exit_NG%
