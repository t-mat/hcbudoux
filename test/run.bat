@echo off && setlocal EnableDelayedExpansion && cd /d "%~dp0" && call "%~dp0..\script\intro.bat"

set "Options=/nologo /utf-8 /Fo:%ProjectRootDir%/_tmp/ /std:c++20 /O2 /EHsc /I ..\include"

echo %MSVC% %Options% test1.c
call %MSVC% %Options% test1.c   || goto :ERROR
                    .\test1.exe || goto :ERROR

echo %MSVC% %Options% test2.cpp
call %MSVC% %Options% test2.cpp || goto :ERROR
                    .\test2.exe || goto :ERROR

:OK
%Exit_OK%

:ERROR
%Exit_NG%
