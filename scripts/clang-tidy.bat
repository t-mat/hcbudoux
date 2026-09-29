@echo off
REM SPDX-License-Identifier: 0BSD
REM
REM clang-tidy.bat - run the project-local LLVM 22.1.3 clang-tidy.
REM
REM Bootstraps via _llvm-ensure.bat on first use (downloads + extracts the
REM exe from the official github.com/llvm/llvm-project release tarball).
REM The system / VS-bundled clang-tidy is not used.
REM
REM Usage:  scripts\clang-tidy.bat <clang-tidy args...>
REM
setlocal
set "HERE=%~dp0"
set "REPO=%HERE%.."
set "CT_EXE=%REPO%\.llvm\bin\clang-tidy.exe"

if not exist "%CT_EXE%" (
    call "%HERE%_llvm-ensure.bat"
    if errorlevel 1 exit /b 1
)

"%CT_EXE%" %*
