@echo off
chcp 65001 >nul
echo ========================================
echo    Unreal MCP Server Status Check
echo ========================================

echo [INFO] Checking current running MCP server processes:
echo.

:: Check each server process
set "running_count=0"

:: Check UMG MCP
tasklist /fi "WINDOWTITLE eq UMG MCP Server*" /fo csv | find "python.exe" >nul
if %errorlevel% equ 0 (
    echo [RUNNING] UMG MCP Server
    set /a running_count+=1
) else (
    echo [STOPPED] UMG MCP Server
)

:: Check Blueprint MCP
tasklist /fi "WINDOWTITLE eq Blueprint MCP Server*" /fo csv | find "python.exe" >nul
if %errorlevel% equ 0 (
    echo [RUNNING] Blueprint MCP Server
    set /a running_count+=1
) else (
    echo [STOPPED] Blueprint MCP Server
)

:: Check Editor MCP
tasklist /fi "WINDOWTITLE eq Editor MCP Server*" /fo csv | find "python.exe" >nul
if %errorlevel% equ 0 (
    echo [RUNNING] Editor MCP Server
    set /a running_count+=1
) else (
    echo [STOPPED] Editor MCP Server
)

:: Check Node MCP
tasklist /fi "WINDOWTITLE eq Node MCP Server*" /fo csv | find "python.exe" >nul
if %errorlevel% equ 0 (
    echo [RUNNING] Node MCP Server
    set /a running_count+=1
) else (
    echo [STOPPED] Node MCP Server
)

:: Check DataTable MCP
tasklist /fi "WINDOWTITLE eq DataTable MCP Server*" /fo csv | find "python.exe" >nul
if %errorlevel% equ 0 (
    echo [RUNNING] DataTable MCP Server
    set /a running_count+=1
) else (
    echo [STOPPED] DataTable MCP Server
)

:: Check Project MCP
tasklist /fi "WINDOWTITLE eq Project MCP Server*" /fo csv | find "python.exe" >nul
if %errorlevel% equ 0 (
    echo [RUNNING] Project MCP Server
    set /a running_count+=1
) else (
    echo [STOPPED] Project MCP Server
)

:: Check Blueprint Action MCP
tasklist /fi "WINDOWTITLE eq Blueprint Action MCP Server*" /fo csv | find "python.exe" >nul
if %errorlevel% equ 0 (
    echo [RUNNING] Blueprint Action MCP Server
    set /a running_count+=1
) else (
    echo [STOPPED] Blueprint Action MCP Server
)

echo.
echo [SUMMARY] Status Summary:
if %running_count% equ 7 (
    echo [SUCCESS] All 7 MCP servers are running normally
) else if %running_count% gtr 0 (
    echo [WARNING] %running_count% servers running, %running_count% stopped
) else (
    echo [ERROR] No MCP servers are running
)

echo.
echo [TIPS] Commands:
echo - Run start_mcp_servers.bat to start all servers
echo - Run stop_mcp_servers.bat to stop all servers
pause
