@echo off
chcp 65001 >nul
echo.
echo ==========================================
echo    🚀 Unreal MCP 一键安装器
echo ==========================================
echo.

REM 检查 Python 是否安装
python --version >nul 2>&1
if errorlevel 1 (
    echo ❌ 错误: 未找到 Python
    echo    请先安装 Python 3.8+ 
    echo    下载地址: https://python.org
    pause
    exit /b 1
)

echo ✅ Python 已安装
python --version

echo.
echo 请输入你的 UE 项目路径（拖拽项目文件夹到此窗口）：
set /p PROJECT_PATH=

if "%PROJECT_PATH%"=="" (
    echo ❌ 未指定项目路径
    pause
    exit /b 1
)

REM 去除路径两端的引号
set PROJECT_PATH=%PROJECT_PATH:"=%

echo.
echo 🔍 检查项目路径: %PROJECT_PATH%

REM 检查是否是有效的 UE 项目
if not exist "%PROJECT_PATH%\*.uproject" (
    echo ❌ 错误: 在指定路径中未找到 .uproject 文件
    echo    请确认路径正确
    pause
    exit /b 1
)

echo ✅ 找到 UE 项目

echo.
echo ==========================================
echo    📦 开始安装 Unreal MCP 工具集
echo ==========================================
echo.

REM 运行安装脚本
python install_to_project.py "%PROJECT_PATH%"

if errorlevel 1 (
    echo.
    echo ❌ 安装失败
    pause
    exit /b 1
)

echo.
echo ==========================================
echo    📚 安装 Python 依赖
echo ==========================================
echo.

cd /d "%PROJECT_PATH%\Python"
if errorlevel 1 (
    echo ❌ 错误: 无法进入 Python 目录
    pause
    exit /b 1
)

echo 正在安装 Python 依赖...
pip install -r requirements.txt

if errorlevel 1 (
    echo ❌ 依赖安装失败，请手动运行:
    echo    cd "%PROJECT_PATH%\Python"
    echo    pip install -r requirements.txt
) else (
    echo ✅ Python 依赖安装成功
)

echo.
echo ==========================================
echo    🎉 安装完成！
echo ==========================================
echo.
echo ✅ Unreal MCP 工具集已成功安装到你的项目
echo.
echo 📋 下一步操作：
echo    1. 重新启动 Unreal Engine 并打开项目
echo    2. 确认 UnrealMCP 插件已加载
echo    3. 运行启动脚本：
echo       "%PROJECT_PATH%\start_mcp_servers.bat"
echo    4. 在 Cursor 中开始使用！
echo.
echo 📖 查看详细说明：
echo    "%PROJECT_PATH%\QUICK_START.md"
echo.
echo 🚀 开始你的 AI 驱动的 UE 开发之旅！
echo.

pause
