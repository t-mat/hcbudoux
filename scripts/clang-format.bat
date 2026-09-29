@echo off
setlocal
set "HERE=%~dp0"
set "REPO=%HERE%.."
set "CF_EXE=%REPO%\.llvm\bin\clang-format.exe"

if not exist "%CF_EXE%" (
    call "%HERE%_llvm-ensure.bat"
    if errorlevel 1 exit /b 1
)

"%CF_EXE%" %*
