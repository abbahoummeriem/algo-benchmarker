@echo off
rem Build script for Windows (GCC / MinGW or MSVC)

if not exist bin mkdir bin
if not exist plots mkdir plots

where gcc >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [BUILD] Compiling with GCC...
    gcc -std=c11 -O3 -Wall -Wextra -Iinclude src\algorithms.c src\benchmark.c src\main.c -o bin\algo_benchmarker.exe
    if %ERRORLEVEL% EQU 0 (
        echo [BUILD SUCCESS] Binary created at bin\algo_benchmarker.exe
        exit /b 0
    )
)

where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [BUILD] Compiling with MSVC cl.exe...
    cl /O2 /W3 /Iinclude src\algorithms.c src\benchmark.c src\main.c /Febin\algo_benchmarker.exe /link Shell32.lib
    if %ERRORLEVEL% EQU 0 (
        echo [BUILD SUCCESS] Binary created at bin\algo_benchmarker.exe
        del *.obj
        exit /b 0
    )
)

echo [ERROR] Neither GCC nor MSVC (cl) found in PATH.
exit /b 1
