for /F "delims=#" %%E in ('"prompt #$E# & for %%E in (1) do rem"') do set "_ESC=%%E"
set "Exit_OK=echo Status = %_ESC%[92m OK %_ESC%[0m && exit /b 0"
set "Exit_NG=echo Status = %_ESC%[91m NG %_ESC%[0m && exit /b 1"

set "ProjectRootDir=%~dp0.."

set "TmpDir=%ProjectRootDir%\_tmp\"

if not exist "%TmpDir%" ( mkdir "%TmpDir%" )

set "ClExeBat=%ProjectRootDir%\script\cl-exe.bat"

set "ClOptions=/nologo /utf-8 /Fo:%TmpDir%"

set "MSVC=%ClExeBat% %ClOptions%"
