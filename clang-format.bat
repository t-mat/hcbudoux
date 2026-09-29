@echo off && setlocal EnableDelayedExpansion && cd /d "%~dp0" && call "%~dp0scripts\intro.bat"

set "ClangFormat=%~dp0scripts\clang-format.bat"

echo clang-format -i codegen\codegen.cpp codegen\hcbudoux.template.h
call "%ClangFormat%" -i codegen\codegen.cpp codegen\hcbudoux.template.h || goto :ERROR

echo clang-format -i examples\example1.c examples\example2.c
call "%ClangFormat%" -i examples\example1.c examples\example2.c || goto :ERROR

echo clang-format -i test\test1.c test\test2.cpp
call "%ClangFormat%" -i test\test1.c test\test2.cpp || goto :ERROR

:OK
%Exit_OK%

:ERROR
%Exit_NG%
