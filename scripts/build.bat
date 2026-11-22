@echo off
cd /d "%~dp0.."
echo ========================================================
echo   Compiling Pure C11 Backend (GU RideShare Engine)
echo ========================================================
if not exist "build" mkdir "build"
if not exist "data" mkdir "data"

gcc -Wall -Wextra -std=c11 -O2 -Iinclude src\main.c src\storage.c src\algorithms.c -o build\rideshare.exe
if %errorlevel% neq 0 (
    echo [ERROR] C compilation failed!
    pause
    exit /b %errorlevel%
)

echo.
echo Running Automated Lab Self-Tests...
build\rideshare.exe --self-test
if %errorlevel% neq 0 (
    echo [ERROR] Self-tests failed!
    pause
    exit /b %errorlevel%
)

echo.
echo ========================================================
echo   Starting GU RideShare Web Dashboard
echo   Open in browser: http://127.0.0.1:8080
echo ========================================================
python web\server.py
pause
