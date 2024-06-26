@echo off
setlocal enabledelayedexpansion

g++ --version

for %%f in ("%~dp0..") do set root=%%~ff
echo Got root of repository: %root%

if not exist %root%\build mkdir %root%\build
set before=-O3 -Wall -I %root%\extern\regex\include -I %root%\src\include -std=c++17
set after=-l pathcch -l wininet

if "%1"=="debug" (
    set before=-D DEBUG -g %before%
    set debug=true
    echo Building in debug mode
) else (
    set debug=false
)

if "%1"=="assembly" (
    set before=-S -fverbose-asm %before%
    set assembly=true
    echo Compiling to assembly
) else (
    set assembly=false
)

if "%assembly%" == "true" (
    set extension=asm
) else (
    set extension=exe
)

echo Building %root%\src\shell.cpp to %root%\build\shell.%extension%
g++ %before% %root%\src\shell.cpp %after% -o %root%\build\shell.%extension%
if %errorlevel% neq 0 exit /b %errorlevel%

for %%f in (%root%\src\external\*) do (
    if "%%~xf" == ".cpp" (
        echo Building %%f to %root%\build\%%~nf.%extension%
        g++ %before% %%f %after% -o %root%\build\%%~nf.%extension%
        if !errorlevel! neq 0 exit /b !errorlevel!
    )
)
