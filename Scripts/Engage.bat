@echo off
chcp 65001 >nul
echo.
echo ===================================================
echo           NEXUS ENGAGED - STARFLEET PROTOCOL
echo           Invoking `Engage.ps1` (canonical Windows harness)
echo ===================================================
echo.

REM Resolve script directory and call the PowerShell harness
SET SCRIPT_DIR=%~dp0
REM Prefer the built-in Windows PowerShell if present
SET "POWERSHELL_EXE=%SystemRoot%\system32\WindowsPowerShell\v1.0\powershell.exe"
IF NOT EXIST "%POWERSHELL_EXE%" (
    REM Try PowerShell Core (pwsh) if available on PATH
    WHERE pwsh >nul 2>nul
    IF %ERRORLEVEL%==0 (
        SET "POWERSHELL_EXE=pwsh"
    ) ELSE (
        REM Fallback: leave POWERSHELL_EXE as-is; the script will print an informative error
    )
)

"%POWERSHELL_EXE%" -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%Engage.ps1" %*
SET RESULT=%ERRORLEVEL%
exit /b %RESULT%