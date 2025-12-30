@echo off
title Banking System Builder
echo ========================================
echo    Banking System Build Script
echo ========================================
echo.

:: Create directories
if not exist data mkdir data
if not exist src (
    echo ERROR: src directory not found!
    echo Please make sure all source files are in the src directory.
    pause
    exit /b 1
)

:: Check for compilers
echo Checking for available C++ compilers...

:: Try Microsoft CL compiler
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo Found: Microsoft Visual C++ Compiler
    echo Compiling banking system...
    cl /EHsc /std:c++14 /I src /O2 /W3 src\main.cpp src\banking.cpp src\utils.cpp src\security.cpp src\file_operations.cpp /Febanking_system.exe
    goto :check_build
)

:: Try GCC compiler
where g++ >nul 2>nul
if %errorlevel% equ 0 (
    echo Found: GCC Compiler
    echo Compiling banking system...
    g++ -std=c++11 -I src -O2 -Wall src\main.cpp src\banking.cpp src\utils.cpp src\security.cpp src\file_operations.cpp -o banking_system.exe
    goto :check_build
)

:: No compiler found
echo.
echo ERROR: No C++ compiler found!
echo.
echo Please install one of the following:
echo 1. Visual Studio Build Tools (free)
echo 2. MinGW-w64
echo.
echo Download Visual Studio Build Tools from:
echo https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
echo.
pause
exit /b 1

:check_build
if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo    BUILD SUCCESSFUL!
    echo ========================================
    echo.
    echo Starting Banking System...
    timeout /t 2 >nul
    banking_system.exe
) else (
    echo.
    echo ========================================
    echo    BUILD FAILED!
    echo ========================================
    echo.
    echo If you don't have a compiler, let's try a different approach...
    pause
    goto :download_compiler
)

:download_compiler
echo.
echo Would you like to install a compiler automatically? (Y/N)
set /p choice=
if /i "%choice%"=="Y" (
    echo Opening compiler download page...
    start https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
)