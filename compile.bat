@echo off
echo Compiling for Windows...
g++ main.cpp database.cpp -o main.exe
if %errorlevel% neq 0 (
    echo Compilation failed.
    pause
    exit /b 1
)
echo Compilation successful. Run with: .\main.exe
pause