@echo off
setlocal
set "HERE=%~dp0"
set "REPO=%HERE%.."
set "LLVM_DIR=%REPO%\.llvm"
set "LLVM_BIN=%LLVM_DIR%\bin"
set "CF_EXE=%LLVM_BIN%\clang-format.exe"
set "CT_EXE=%LLVM_BIN%\clang-tidy.exe"

REM Fast path: both exes present -> nothing to do.
if exist "%CF_EXE%" if exist "%CT_EXE%" exit /b 0

if not exist "%LLVM_BIN%" mkdir "%LLVM_BIN%"

set "TARBALL=%LLVM_DIR%\llvm-23.1.2.tar.xz"
set "URL=https://github.com/llvm/llvm-project/releases/download/llvmorg-23.1.2/clang+llvm-23.1.2-x86_64-pc-windows-msvc.tar.xz"
set "TOPDIR=clang+llvm-23.1.2-x86_64-pc-windows-msvc"

if not exist "%TARBALL%" (
    echo [_llvm-ensure.bat] downloading LLVM 23.1.2 tarball 1>&2
    curl.exe -sSfL -o "%TARBALL%" "%URL%"
    if errorlevel 1 exit /b 1
)

echo [_llvm-ensure.bat] extracting clang-format.exe and clang-tidy.exe 1>&2
tar.exe -xJf "%TARBALL%" --strip-components=2 -C "%LLVM_BIN%" "%TOPDIR%/bin/clang-format.exe" "%TOPDIR%/bin/clang-tidy.exe"
if errorlevel 1 exit /b 1

REM Free the 862 MB tarball; re-download only needed if user wipes .llvm\bin.
del /q "%TARBALL%" 2>nul

if not exist "%CF_EXE%" (
    echo [_llvm-ensure.bat] clang-format.exe missing after extract 1>&2
    exit /b 1
)
if not exist "%CT_EXE%" (
    echo [_llvm-ensure.bat] clang-tidy.exe missing after extract 1>&2
    exit /b 1
)

exit /b 0
