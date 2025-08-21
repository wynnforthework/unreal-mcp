@echo off
chcp 65001 >nul
echo ========================================
echo    Stop All Unreal MCP Servers
echo ========================================

echo [INFO] Stopping all MCP server processes...

:: Stop all MCP related Python processes
taskkill /f /im python.exe /fi "WINDOWTITLE eq *MCP*" >nul 2>&1
taskkill /f /im uv.exe >nul 2>&1

:: Wait for processes to fully stop
timeout /t 2 /nobreak >nul

echo [SUCCESS] All MCP servers stopped
echo.
echo [TIP] Run start_mcp_servers.bat to restart servers
pause
