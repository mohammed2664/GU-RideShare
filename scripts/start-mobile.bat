@echo off
cd /d "%~dp0.."
set HOST=0.0.0.0
set PORT=8080
echo ========================================================
echo   Starting GU RideShare for Mobile / Local Wi-Fi
echo ========================================================
echo Make sure your phone is connected to the same Wi-Fi network.
echo Find your laptop IP using: ipconfig
echo On your mobile browser, visit: http://YOUR-LAPTOP-IP:8080
echo ========================================================
python web\server.py
pause
