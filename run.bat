@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"
call "%~dp0scripts\intro.bat"

rem run.bat - Windows counterpart of the Makefile targets.
rem
rem   run.bat [target ...]
rem
rem   (none)        same as "run"
rem   run           build and run tests and examples
rem   all           clean, codegen, test, examples
rem   clean         delete build outputs (_tmp\*.obj and the .exe files)
rem   codegen       build and run codegen (regenerates include\hcbudoux.h)
rem   test          build and run test\test1 (C) and test\test2 (C++)
rem   examples      build and run examples\example1 and examples\example2
rem   clang-format  format the sources with the project-local clang-format (scripts\clang-format.bat)
rem   clang-tidy    analyze the sources with the project-local clang-tidy (scripts\clang-tidy.bat)
rem   help          print this list
rem
rem Targets run in the order given and stop at the first failure, like make.

if "%~1"=="" (
  call :run || goto :ERROR
  goto :OK
)

:NextTarget
if "%~1"=="" goto :OK
set "Target=%~1"
set "Known="
for %%T in (run all clean codegen test examples clang-format clang-tidy help) do (
  if /I "%Target%"=="%%T" set "Known=1"
)
if not defined Known (
  echo run.bat: unknown target "%Target%"
  call :help
  goto :ERROR
)
call :%Target% || goto :ERROR
shift
goto :NextTarget

:OK
%Exit_OK%

:ERROR
%Exit_NG%

rem ------------------------------------------------------------------ targets

:run
call :test || exit /b 1
call :examples || exit /b 1
exit /b 0

:all
call :clean || exit /b 1
call :codegen || exit /b 1
call :test || exit /b 1
call :examples || exit /b 1
exit /b 0

:clean
echo clean
del /q "_tmp\*.obj" 2>nul
del /q "codegen\codegen.exe" "test\test1.exe" "test\test2.exe" "examples\example1.exe" "examples\example2.exe" 2>nul
exit /b 0

:codegen
call "codegen\run.bat"
exit /b %errorlevel%

:test
call "test\run.bat"
exit /b %errorlevel%

:examples
call "examples\run.bat"
exit /b %errorlevel%

:clang-format
set "ClangFormat=%~dp0scripts\clang-format.bat"
echo clang-format -i codegen\codegen.cpp codegen\hcbudoux.template.h
call "%ClangFormat%" -i codegen\codegen.cpp codegen\hcbudoux.template.h || exit /b 1
echo clang-format -i examples\example1.c examples\example2.c
call "%ClangFormat%" -i examples\example1.c examples\example2.c || exit /b 1
echo clang-format -i test\test1.c test\test2.cpp
call "%ClangFormat%" -i test\test1.c test\test2.cpp || exit /b 1
exit /b 0

:clang-tidy
set "ClangTidy=%~dp0scripts\clang-tidy.bat"
rem The check set comes from .clang-tidy at the repository root (same as `make clang-tidy`).
set "Warnings=-Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wshadow -Wswitch-enum -Wundef -Wpointer-arith -Wstrict-aliasing=1"
set "CFlags=-I include -std=c11 %Warnings% -Wstrict-prototypes"
rem The MSVC standard library requires C++14, so use c++14 here like codegen\run.bat
rem (the Makefile uses -std=c++11 with libstdc++).
set "CxxFlags=-I include -I third_party\json.h -std=c++14 %Warnings%"
echo clang-tidy codegen\codegen.cpp -header-filter= -- %CxxFlags%
call "%ClangTidy%" codegen\codegen.cpp -header-filter= -- %CxxFlags% || exit /b 1
echo clang-tidy codegen\hcbudoux.template.c -- %CFlags%
call "%ClangTidy%" codegen\hcbudoux.template.c -- %CFlags% || exit /b 1
echo clang-tidy examples\example1.c -- %CFlags%
call "%ClangTidy%" examples\example1.c -- %CFlags% || exit /b 1
echo clang-tidy examples\example2.c -- %CFlags%
call "%ClangTidy%" examples\example2.c -- %CFlags% || exit /b 1
echo clang-tidy test\test1.c -- %CFlags%
call "%ClangTidy%" test\test1.c -- %CFlags% || exit /b 1
exit /b 0

:help
echo Usage: run.bat [target ...]
echo.
echo   (none)        same as "run"
echo   run           build and run tests and examples
echo   all           clean, codegen, test, examples
echo   clean         delete build outputs (_tmp\*.obj and the .exe files)
echo   codegen       build and run codegen (regenerates include\hcbudoux.h)
echo   test          build and run test\test1 (C) and test\test2 (C++)
echo   examples      build and run examples\example1 and examples\example2
echo   clang-format  format the sources with the project-local clang-format
echo   clang-tidy    analyze the sources with the project-local clang-tidy
echo   help          print this list
echo.
echo Targets run in the order given and stop at the first failure, like make.
exit /b 0
