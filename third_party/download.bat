@rem @echo off
setlocal enabledelayedexpansion
set /a errorno=1

cd /d "%~dp0"

set /p json_h_hash=<json_h_hash.txt
set /p budoux_hash=<budoux_hash.txt

cd ..

if not exist "_tmp" ( md "_tmp" >nul 2>&1 )
rmdir /Q /S third_party\budoux >nul 2>&1
md          third_party\budoux\budoux\models >nul 2>&1
rmdir /Q /S third_party\json.h >nul 2>&1
md          third_party\json.h >nul 2>&1
rmdir /Q /S _tmp\staging >nul 2>&1
md          _tmp\staging >nul 2>&1

pushd "_tmp\staging" || goto :ERROR

curl -JLo sheredom-json.h.zip https://github.com/sheredom/json.h/archive/%json_h_hash%.zip
tar xvf sheredom-json.h.zip || goto :ERROR
ren json.h-%json_h_hash% json.h || goto :ERROR

curl -JLo google-budoux.zip https://github.com/google/budoux/archive/%budoux_hash%.zip
tar xvf google-budoux.zip || goto :ERROR
ren budoux-%budoux_hash% budoux || goto :ERROR

popd

copy _tmp\staging\json.h\LICENSE              third_party\json.h\               || goto :ERROR
copy _tmp\staging\json.h\json.h               third_party\json.h\               || goto :ERROR
copy _tmp\staging\budoux\LICENSE              third_party\budoux\               || goto :ERROR
copy _tmp\staging\budoux\budoux\models\*.json third_party\budoux\budoux\models\ || goto :ERROR

rmdir /Q /S _tmp\staging >nul 2>&1 || goto :ERROR

set /a errorno=0
goto :END

:ERROR

echo ERROR

:END
exit /B %errorno%
