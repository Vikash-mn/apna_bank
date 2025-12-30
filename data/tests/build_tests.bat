@echo off
title Banking System Tests
echo Building test suite...

:: Try different compilers
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo Using Microsoft CL compiler...
    cl /EHsc /std:c++14 /I ../src /O2 test_banking.cpp ../src/utils.cpp ../src/security.cpp ../src/file_operations.cpp /Fetest_banking.exe
) else (
    where g++ >nul 2>nul
    if %errorlevel% equ 0 (
        echo Using GCC compiler...
        g++ -std=c++11 -I ../src -O2 test_banking.cpp ../src/utils.cpp ../src/security.cpp ../src/file_operations.cpp -o test_banking.exe
    ) else (
        echo No compiler found!
        pause
        exit /b 1
    )
)

if %errorlevel% equ 0 (
    echo.
    echo Running tests...
    test_banking.exe
) else (
    echo Build failed!
)

pause