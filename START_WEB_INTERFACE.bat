@echo off
chcp 65001 >nul
title Unreal MCP Web Interface Launcher

echo =====================================
echo   Unreal MCP Web 界面启动器
echo =====================================
echo.

:: 检查 Python 是否安装
python --version >nul 2>&1
if errorlevel 1 (
    echo ❌ 错误: 未找到 Python!
    echo    请先安装 Python 3.7+ 并添加到 PATH
    echo    下载地址: https://www.python.org/downloads/
    pause
    exit /b 1
)

echo ✅ Python 已安装
python --version

:: 检查 web-interface 目录
if not exist "web-interface" (
    echo ❌ 错误: 未找到 web-interface 目录!
    echo    请确保在 unreal-mcp 项目根目录运行此脚本
    pause
    exit /b 1
)

echo ✅ Web interface 目录存在

:: 切换到 web-interface 目录
cd web-interface

:: 检查依赖
echo.
echo 📦 检查 Python 依赖...

:: 检查 Flask 是否安装
python -c "import flask" >nul 2>&1
if errorlevel 1 (
    echo ⚠️  Flask 未安装，正在安装依赖...
    pip install flask flask-socketio eventlet
    if errorlevel 1 (
        echo ❌ 依赖安装失败!
        pause
        exit /b 1
    )
    echo ✅ 依赖安装完成
) else (
    echo ✅ 依赖已存在
)

:: 检查应用文件
if exist "app_enhanced.py" (
    set APP_FILE=app_enhanced.py
    echo ✅ 使用增强版 Web 界面
) else if exist "app.py" (
    set APP_FILE=app.py
    echo ✅ 使用标准 Web 界面
) else (
    echo ❌ 错误: 未找到 Web 应用文件!
    pause
    exit /b 1
)

echo.
echo 🚀 启动 Unreal MCP Web 界面...
echo    访问地址: http://localhost:5000
echo    按 Ctrl+C 停止服务器
echo.
echo =====================================

:: 启动 Web 服务器
python %APP_FILE%

:: 如果脚本到达这里，说明服务器已停止
echo.
echo 🛑 Web 界面已停止
pause
