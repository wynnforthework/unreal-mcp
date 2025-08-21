@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo    Unreal MCP Server Manager
echo ========================================

:: Set Python directory
set PYTHON_DIR=D:\UGit\unreal-mcp\Python
cd /d "%PYTHON_DIR%"

:: Check parameters
if "%1"=="stop" goto :stop_servers
if "%1"=="status" goto :check_status

:: Default: start servers
goto :start_servers

:start_servers
echo [INFO] Checking existing MCP server processes...

:: Stop existing MCP server processes
echo [INFO] Stopping existing processes...
taskkill /f /im python.exe /fi "WINDOWTITLE eq *MCP*" >nul 2>&1
taskkill /f /im uv.exe >nul 2>&1
timeout /t 2 /nobreak >nul

echo.
echo [INFO] Starting all MCP servers...

:: Start all MCP servers
start "UMG MCP Server" cmd /k "title UMG MCP Server && echo Starting UMG MCP Server... && uv run umg_mcp_server.py"
timeout /t 1 /nobreak >nul

start "Blueprint MCP Server" cmd /k "title Blueprint MCP Server && echo Starting Blueprint MCP Server... && uv run blueprint_mcp_server.py"
timeout /t 1 /nobreak >nul

start "Editor MCP Server" cmd /k "title Editor MCP Server && echo Starting Editor MCP Server... && uv run editor_mcp_server.py"
timeout /t 1 /nobreak >nul

start "Node MCP Server" cmd /k "title Node MCP Server && echo Starting Node MCP Server... && uv run node_mcp_server.py"
timeout /t 1 /nobreak >nul

start "DataTable MCP Server" cmd /k "title DataTable MCP Server && echo Starting DataTable MCP Server... && uv run datatable_mcp_server.py"
timeout /t 1 /nobreak >nul

start "Project MCP Server" cmd /k "title Project MCP Server && echo Starting Project MCP Server... && uv run project_mcp_server.py"
timeout /t 1 /nobreak >nul

start "Blueprint Action MCP Server" cmd /k "title Blueprint Action MCP Server && echo Starting Blueprint Action MCP Server... && uv run blueprint_action_mcp_server.py"

echo.
echo [SUCCESS] All MCP servers started successfully!
echo.
echo [USAGE] Instructions:
echo   - Closing this window will NOT stop the servers
echo   - Run "%~f0 stop" to stop all servers
echo   - Run "%~f0 status" to check server status
echo.
echo [READY] You can now use Unreal MCP tools in your AI assistant!
echo.
pause
goto :end

:stop_servers
echo [INFO] Stopping all MCP servers...
taskkill /f /im python.exe /fi "WINDOWTITLE eq *MCP*" >nul 2>&1
taskkill /f /im uv.exe >nul 2>&1
echo [SUCCESS] All MCP servers stopped
pause
goto :end

:check_status
echo [INFO] Checking MCP server status...
echo.
echo Current running MCP server processes:
echo -------------------------------------

:: Check server processes
set "found=0"

:: Check all MCP related processes
tasklist /fi "WINDOWTITLE eq *MCP*" /fo csv | find "python.exe" >nul
if !errorlevel! equ 0 (
    echo [FOUND] Running MCP server processes detected
    set "found=1"
) else (
    echo [NOT FOUND] No running MCP server processes
)

echo.
if !found! equ 1 (
    echo [STATUS] Some servers are running
) else (
    echo [STATUS] No servers are running
)
pause
goto :end

:end
endlocal
