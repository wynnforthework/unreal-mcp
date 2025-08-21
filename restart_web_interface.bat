@echo off
echo 重启Web界面服务器...

cd /d "%~dp0web-interface"

echo 停止现有服务器...
taskkill /F /IM python.exe 2>nul
timeout /t 2 /nobreak >nul

echo 启动Web界面服务器...
start "Web Interface" python app_enhanced.py

echo.
echo ========================================
echo Web界面服务器已重启！
echo 访问地址: http://localhost:5000
echo.
echo 🆕 新功能: 重新安装MCP工具
echo - 已安装MCP工具的项目现在显示"更新MCP"按钮
echo - 点击"更新MCP"可以重新安装/更新MCP工具
echo - 重新安装会覆盖现有文件并更新Python依赖
echo ========================================
pause
