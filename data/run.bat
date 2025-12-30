@echo off
echo Building Banking System...
g++ -std=c++11 -Wall -Wextra -O2 -I./src src/main.cpp src/banking.cpp src/utils.cpp src/security.cpp src/file_operations.cpp -o banking_system.exe

if %errorlevel% equ 0 (
    echo Build successful! Starting application...
    banking_system.exe
) else (
    echo Build failed!
    pause
    exit /b 1
)