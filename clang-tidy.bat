@echo off && setlocal EnableDelayedExpansion && cd /d "%~dp0" && call "%~dp0scripts\intro.bat"

set "ClangTidy=%~dp0scripts\clang-tidy.bat"

rem LLVM 23 enables only clang-diagnostic-* by default and exits with
rem "no checks enabled".  Request the classic default set explicitly.
set "Checks=-checks=clang-diagnostic-*,clang-analyzer-*"

set "Warnings=-Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wshadow -Wswitch-enum -Wundef -Wpointer-arith -Wstrict-aliasing=1"
set "CFlags=-I include -std=c11 %Warnings% -Wstrict-prototypes"
rem The MSVC standard library requires C++14, so use c++14 here like codegen\run.bat
rem (the Makefile uses -std=c++11 with libstdc++).
set "CxxFlags=-I include -I third_party\json.h -std=c++14 %Warnings%"

echo clang-tidy %Checks% codegen\codegen.cpp -header-filter= -- %CxxFlags%
call "%ClangTidy%" "%Checks%" codegen\codegen.cpp -header-filter= -- %CxxFlags% || goto :ERROR

echo clang-tidy %Checks% codegen\hcbudoux.template.c -- %CFlags%
call "%ClangTidy%" "%Checks%" codegen\hcbudoux.template.c -- %CFlags% || goto :ERROR

echo clang-tidy %Checks% examples\example1.c -- %CFlags%
call "%ClangTidy%" "%Checks%" examples\example1.c -- %CFlags% || goto :ERROR

echo clang-tidy %Checks% examples\example2.c -- %CFlags%
call "%ClangTidy%" "%Checks%" examples\example2.c -- %CFlags% || goto :ERROR

echo clang-tidy %Checks% test\test1.c -- %CFlags%
call "%ClangTidy%" "%Checks%" test\test1.c -- %CFlags% || goto :ERROR

:OK
%Exit_OK%

:ERROR
%Exit_NG%
