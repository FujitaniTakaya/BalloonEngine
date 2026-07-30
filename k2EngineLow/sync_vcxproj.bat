@echo off
chcp 65001 >nul
cd /d "%~dp0"

where python >nul 2>nul
if %errorlevel%==0 (
    python -X utf8 ..\tools\sync_vcxproj.py k2EngineLow %*
) else (
    py -X utf8 ..\tools\sync_vcxproj.py k2EngineLow %*
)

echo.
pause
