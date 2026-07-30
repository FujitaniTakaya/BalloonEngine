@echo off
chcp 65001 >nul
cd /d "%~dp0"

where python >nul 2>nul
if %errorlevel%==0 (
    python -X utf8 ..\tools\sync_vcxproj.py Game %*
) else (
    py -X utf8 ..\tools\sync_vcxproj.py Game %*
)

echo.
pause
