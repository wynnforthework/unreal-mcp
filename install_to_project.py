#!/usr/bin/env python3
"""
Unreal MCP 完整工具集安装脚本

将 Unreal MCP 完整工具集安装到现有的 Unreal Engine 项目中
包括：UnrealMCP插件、所有MCP工具、Agent系统等
"""

import os
import sys
import shutil
import json
from pathlib import Path
from typing import Optional


def find_ue_project(search_path: str) -> Optional[Path]:
    """查找UE项目文件"""
    search_dir = Path(search_path)
    
    # 在当前目录查找
    uproject_files = list(search_dir.glob("*.uproject"))
    if uproject_files:
        return search_dir
    
    # 在子目录中查找
    for subdir in search_dir.iterdir():
        if subdir.is_dir():
            uproject_files = list(subdir.glob("*.uproject"))
            if uproject_files:
                return subdir
    
    return None


def copy_unreal_mcp_plugin(source_dir: Path, target_project: Path) -> bool:
    """复制UnrealMCP插件到目标项目"""
    
    # 查找源插件目录
    source_plugin = source_dir / "MCPGameProject" / "Plugins" / "UnrealMCP"
    if not source_plugin.exists():
        print(f"❌ 未找到源插件目录: {source_plugin}")
        return False
    
    # 目标插件目录
    target_plugins = target_project / "Plugins"
    target_plugins.mkdir(exist_ok=True)
    target_plugin = target_plugins / "UnrealMCP"
    
    # 复制插件
    if target_plugin.exists():
        print(f"⚠️  插件目录已存在，正在覆盖: {target_plugin}")
        shutil.rmtree(target_plugin)
    
    shutil.copytree(source_plugin, target_plugin)
    print(f"✅ UnrealMCP插件已复制到: {target_plugin}")
    
    return True


def copy_mcp_tools(source_dir: Path, target_project: Path) -> bool:
    """复制所有MCP工具到目标项目"""
    
    # 需要复制的MCP服务器和工具
    mcp_components = [
        'umg_mcp_server.py',
        'umg_tools/',
        'blueprint_mcp_server.py', 
        'blueprint_tools/',
        'editor_mcp_server.py',
        'editor_tools/',
        'node_mcp_server.py',
        'node_tools/',
        'datatable_mcp_server.py',
        'datatable_tools/',
        'project_mcp_server.py',
        'project_tools/',
        'blueprint_action_mcp_server.py',
        'blueprint_action_tools/',
        'utils/',
        'pyproject.toml'
    ]
    
    target_python = target_project / "Python"
    target_python.mkdir(exist_ok=True)
    
    success = True
    for component in mcp_components:
        source_path = source_dir / "Python" / component
        target_path = target_python / component
        
        if source_path.exists():
            try:
                if target_path.exists():
                    if target_path.is_dir():
                        shutil.rmtree(target_path)
                    else:
                        target_path.unlink()
                
                if source_path.is_dir():
                    shutil.copytree(source_path, target_path)
                else:
                    shutil.copy2(source_path, target_path)
                
                print(f"✅ 已复制: {component}")
            except Exception as e:
                print(f"❌ 复制 {component} 失败: {e}")
                success = False
        else:
            print(f"⚠️  跳过不存在的组件: {component}")
    
    return success


def copy_agent_system(source_dir: Path, target_project: Path) -> bool:
    """复制Agent系统到目标项目"""
    
    # 查找源agents目录
    source_agents = source_dir / "Python" / "agents"
    if not source_agents.exists():
        print(f"❌ 未找到源agents目录: {source_agents}")
        return False
    
    # 目标Python目录
    target_python = target_project / "Python"
    target_python.mkdir(exist_ok=True)
    target_agents = target_python / "agents"
    
    # 复制agents系统
    if target_agents.exists():
        print(f"⚠️  agents目录已存在，正在覆盖: {target_agents}")
        shutil.rmtree(target_agents)
    
    shutil.copytree(source_agents, target_agents)
    print(f"✅ Agent系统已复制到: {target_agents}")
    
    return True


def update_uproject_file(project_dir: Path) -> bool:
    """更新.uproject文件以包含必要的插件"""
    
    uproject_files = list(project_dir.glob("*.uproject"))
    if not uproject_files:
        print("❌ 未找到.uproject文件")
        return False
    
    uproject_file = uproject_files[0]
    
    try:
        # 读取现有配置
        with open(uproject_file, 'r', encoding='utf-8') as f:
            project_config = json.load(f)
        
        # 确保有Plugins数组
        if "Plugins" not in project_config:
            project_config["Plugins"] = []
        
        plugins = project_config["Plugins"]
        
        # 检查并添加必要的插件
        required_plugins = [
            {"Name": "UnrealMCP", "Enabled": True},
            {"Name": "PythonScriptPlugin", "Enabled": True}
        ]
        
        for required_plugin in required_plugins:
            # 检查插件是否已存在
            existing = next((p for p in plugins if p["Name"] == required_plugin["Name"]), None)
            if existing:
                existing["Enabled"] = True
                print(f"✅ 插件已存在并启用: {required_plugin['Name']}")
            else:
                plugins.append(required_plugin)
                print(f"✅ 添加插件: {required_plugin['Name']}")
        
        # 保存更新的配置
        with open(uproject_file, 'w', encoding='utf-8') as f:
            json.dump(project_config, f, indent=4)
        
        print(f"✅ 项目文件已更新: {uproject_file}")
        return True
        
    except Exception as e:
        print(f"❌ 更新项目文件失败: {e}")
        return False


def create_requirements_file(project_dir: Path) -> bool:
    """创建Python依赖文件"""
    
    requirements_content = """# Unreal MCP 完整工具集依赖
# 注意：此项目使用 uv 作为包管理器
# 安装 uv: https://docs.astral.sh/uv/getting-started/installation/

mcp[cli]>=1.4.1
fastmcp>=0.2.0
uvicorn
fastapi
pydantic>=2.6.1
requests
aiohttp
pyyaml
pytest
pytest-asyncio
"""
    
    python_dir = project_dir / "Python"
    python_dir.mkdir(exist_ok=True)
    
    requirements_file = python_dir / "requirements.txt"
    requirements_file.write_text(requirements_content)
    
    print(f"✅ 依赖文件已创建: {requirements_file}")
    return True


def create_startup_scripts(project_dir: Path) -> bool:
    """创建启动脚本"""
    
    # Windows 启动脚本
    startup_bat = f"""@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo    Unreal MCP Server Manager
echo ========================================

:: Set Python directory
set PYTHON_DIR={project_dir / 'Python'}
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
"""
    
    # 停止脚本
    stop_bat = f"""@echo off
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
"""
    
    # 状态检查脚本
    status_bat = f"""@echo off
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
"""
    
    # 创建所有脚本
    startup_script = project_dir / "start_mcp_servers.bat"
    startup_script.write_text(startup_bat, encoding='utf-8')
    
    stop_script = project_dir / "stop_mcp_servers.bat"
    stop_script.write_text(stop_bat, encoding='utf-8')
    
    status_script = project_dir / "check_mcp_status.bat"
    status_script.write_text(status_bat, encoding='utf-8')
    
    print(f"✅ 启动脚本已创建: {startup_script}")
    print(f"✅ 停止脚本已创建: {stop_script}")
    print(f"✅ 状态检查脚本已创建: {status_script}")
    return True


def create_mcp_config(project_dir: Path) -> bool:
    """创建MCP配置文件"""
    
    # 定义 Unreal MCP 服务器配置
    unreal_mcp_servers = {
        "unrealBlueprintMCP": {
            "command": "uv",
            "args": [
                "--directory",
                str(project_dir / "Python"),
                "run",
                "blueprint_mcp_server.py"
            ]
        },
        "unrealEditorMCP": {
            "command": "uv",
            "args": [
                "--directory",
                str(project_dir / "Python"),
                "run",
                "editor_mcp_server.py"
            ]
        },
        "unrealUMGMCP": {
            "command": "uv",
            "args": [
                "--directory",
                str(project_dir / "Python"),
                "run",
                "umg_mcp_server.py"
            ]
        },
        "unrealNodeMCP": {
            "command": "uv",
            "args": [
                "--directory",
                str(project_dir / "Python"),
                "run",
                "node_mcp_server.py"
            ]
        },
        "unrealDataTableMCP": {
            "command": "uv",
            "args": [
                "--directory",
                str(project_dir / "Python"),
                "run",
                "datatable_mcp_server.py"
            ]
        },
        "unrealProjectMCP": {
            "command": "uv",
            "args": [
                "--directory",
                str(project_dir / "Python"),
                "run",
                "project_mcp_server.py"
            ]
        },
        "unrealBlueprintActionMCP": {
            "command": "uv",
            "args": [
                "--directory",
                str(project_dir / "Python"),
                "run",
                "blueprint_action_mcp_server.py"
            ]
        }
    }
    
    # 为 Cursor 创建配置
    cursor_dir = project_dir / ".cursor"
    cursor_dir.mkdir(exist_ok=True)
    cursor_config = cursor_dir / "mcp.json"
    
    # 检查是否已存在配置文件
    existing_config = {}
    if cursor_config.exists():
        try:
            with open(cursor_config, 'r', encoding='utf-8') as f:
                existing_config = json.load(f)
            print(f"📁 发现现有MCP配置: {cursor_config}")
        except Exception as e:
            print(f"⚠️  读取现有配置失败，将创建新配置: {e}")
            existing_config = {}
    
    # 检查是否已安装过 Unreal MCP
    unreal_mcp_installed = False
    if "mcpServers" in existing_config:
        for server_name in unreal_mcp_servers.keys():
            if server_name in existing_config["mcpServers"]:
                unreal_mcp_installed = True
                break
    
    # 处理不同的安装情况
    if unreal_mcp_installed:
        print("🔄 检测到已安装的 Unreal MCP，正在更新配置...")
        # 更新现有的 Unreal MCP 服务器配置
        if "mcpServers" not in existing_config:
            existing_config["mcpServers"] = {}
        
        for server_name, server_config in unreal_mcp_servers.items():
            existing_config["mcpServers"][server_name] = server_config
            print(f"  ✅ 更新服务器配置: {server_name}")
    else:
        print("🆕 创建新的 Unreal MCP 配置...")
        # 创建新的配置或添加到现有配置
        if "mcpServers" not in existing_config:
            existing_config["mcpServers"] = {}
        
        for server_name, server_config in unreal_mcp_servers.items():
            existing_config["mcpServers"][server_name] = server_config
            print(f"  ✅ 添加服务器配置: {server_name}")
    
    # 保存更新后的配置
    try:
        with open(cursor_config, 'w', encoding='utf-8') as f:
            json.dump(existing_config, f, indent=2)
        print(f"✅ Cursor MCP配置已更新: {cursor_config}")
    except Exception as e:
        print(f"❌ 保存Cursor配置失败: {e}")
        return False
    
    # 创建通用配置文件（用于其他AI助手）
    general_config = project_dir / "mcp_config.json"
    try:
        with open(general_config, 'w', encoding='utf-8') as f:
            json.dump(existing_config, f, indent=2)
        print(f"✅ 通用MCP配置已创建: {general_config}")
    except Exception as e:
        print(f"❌ 保存通用配置失败: {e}")
        return False
    
    return True


def create_quick_start_guide(project_dir: Path) -> bool:
    """创建快速开始指南"""
    
    guide_content = f"""# Unreal MCP 快速开始指南

## 安装完成！

恭喜！Unreal MCP 完整工具集已成功安装到你的项目中。

## 下一步操作

### 1. 安装 Python 依赖
```bash
cd "{project_dir / 'Python'}"
# 使用 uv 安装依赖（推荐）
uv sync

# 或者使用 pip（如果未安装 uv）
pip install -r requirements.txt
```

### 2. 启动 Unreal Engine 项目
- 重新启动 Unreal Engine
- 打开你的项目
- 确认 UnrealMCP 插件已加载

### 3. 启动 MCP 服务器
运行启动脚本：
```bash
{project_dir / 'start_mcp_servers.bat'}
```

或使用参数：
```bash
# 启动所有服务器
{project_dir / 'start_mcp_servers.bat'}

# 停止所有服务器
{project_dir / 'start_mcp_servers.bat'} stop

# 检查服务器状态
{project_dir / 'start_mcp_servers.bat'} status
```

或手动启动：
```bash
cd "{project_dir / 'Python'}"
uv run umg_mcp_server.py
uv run blueprint_mcp_server.py
uv run editor_mcp_server.py
uv run node_mcp_server.py
uv run datatable_mcp_server.py
uv run project_mcp_server.py
uv run blueprint_action_mcp_server.py
```

### 4. 配置 AI 助手

#### Cursor 配置
MCP 配置已自动创建在：`{project_dir / '.cursor' / 'mcp.json'}`

#### Claude Desktop 配置
复制 `{project_dir / 'mcp_config.json'}` 的内容到：
`%USERPROFILE%\\.config\\claude-desktop\\mcp.json`

#### Windsurf 配置
复制 `{project_dir / 'mcp_config.json'}` 的内容到：
`%USERPROFILE%\\.config\\windsurf\\mcp.json`

### 5. 测试功能
在 AI 助手中尝试：
- "创建一个立方体"
- "创建一个基于 Actor 的 Blueprint"
- "列出关卡中的所有 Actor"

## 功能概览

- ✅ UnrealMCP 插件（C++）
- ✅ 完整 MCP 工具集
- ✅ UMG/UI 工具
- ✅ Blueprint 开发工具
- ✅ Editor 控制工具
- ✅ Node/节点工具
- ✅ DataTable 工具
- ✅ Project 管理工具
- ✅ Blueprint Action 工具
- ✅ Agent 自动化系统

## 需要帮助？

查看完整文档：
- 功能总览：`Docs/FEATURES_ZH.md`
- 使用指南：`Docs/USAGE_GUIDE_ZH.md`
- AI 集成指南：`Docs/AI_INTEGRATION_GUIDE_ZH.md`

祝你使用愉快！🎉
"""
    
    guide_file = project_dir / "QUICK_START.md"
    guide_file.write_text(guide_content, encoding='utf-8')
    
    print(f"✅ 快速开始指南已创建: {guide_file}")
    return True


def main():
    """主安装函数"""
    
    print("🚀 Unreal MCP 完整工具集安装器")
    print("=" * 50)
    
    # 获取源目录（当前脚本所在目录）
    source_dir = Path(__file__).parent.absolute()
    print(f"源目录: {source_dir}")
    
    # 获取目标项目目录
    if len(sys.argv) > 1:
        target_path = sys.argv[1]
    else:
        target_path = input("请输入目标UE项目路径: ").strip()
    
    if not target_path:
        print("❌ 未指定目标路径")
        return
    
    # 查找UE项目
    project_dir = find_ue_project(target_path)
    if not project_dir:
        print(f"❌ 在 {target_path} 中未找到UE项目")
        return
    
    print(f"✅ 找到UE项目: {project_dir}")
    
    # 确认安装
    uproject_files = list(project_dir.glob("*.uproject"))
    project_name = uproject_files[0].stem if uproject_files else "Unknown"
    
    print(f"\n即将安装到项目: {project_name}")
    print(f"项目路径: {project_dir}")
    print(f"\n将安装以下组件：")
    print(f"- UnrealMCP C++ 插件")
    print(f"- 完整 MCP 工具集（7个服务器）")
    print(f"- Agent 自动化系统")
    print(f"- 启动脚本和配置文件")
    print(f"- 快速开始指南")
    
    confirm = input("\n是否继续安装? (y/N): ").strip().lower()
    if confirm not in ['y', 'yes']:
        print("安装已取消")
        return
    
    print("\n开始安装...")
    
    # 执行安装步骤
    success = True
    
    # 1. 复制UnrealMCP插件
    print("\n1. 安装UnrealMCP插件...")
    if not copy_unreal_mcp_plugin(source_dir, project_dir):
        success = False
    
    # 2. 复制MCP工具
    print("\n2. 安装MCP工具...")
    if not copy_mcp_tools(source_dir, project_dir):
        success = False
    
    # 3. 复制Agent系统
    print("\n3. 安装Agent系统...")
    if not copy_agent_system(source_dir, project_dir):
        success = False
    
    # 4. 更新项目配置
    print("\n4. 更新项目配置...")
    if not update_uproject_file(project_dir):
        success = False
    
    # 5. 创建依赖文件
    print("\n5. 创建Python依赖文件...")
    if not create_requirements_file(project_dir):
        success = False
    
    # 6. 创建启动脚本
    print("\n6. 创建启动脚本...")
    if not create_startup_scripts(project_dir):
        success = False
    
    # 7. 创建MCP配置
    print("\n7. 创建MCP配置文件...")
    if not create_mcp_config(project_dir):
        success = False
    
    # 8. 创建快速开始指南
    print("\n8. 创建快速开始指南...")
    if not create_quick_start_guide(project_dir):
        success = False
    
    # 安装结果
    print("\n" + "=" * 50)
    if success:
        print("🎉 安装完成!")
        print("\n✅ 已安装的组件：")
        print("  - UnrealMCP C++ 插件")
        print("  - 7个 MCP 服务器（UMG、Blueprint、Editor等）")
        print("  - Agent 自动化系统")
        print("  - 智能启动脚本 (start_mcp_servers.bat)")
        print("  - 停止脚本 (stop_mcp_servers.bat)")
        print("  - 状态检查脚本 (check_mcp_status.bat)")
        print("  - MCP 配置文件")
        print("  - 快速开始指南 (QUICK_START.md)")
        
        print("\n🚀 下一步（最简单的使用方式）：")
        print("1. 重新启动 Unreal Engine 并打开项目")
        print("2. 安装 Python 依赖：")
        print(f"   cd {project_dir / 'Python'}")
        print("   pip install -r requirements.txt")
        print("3. 双击运行启动脚本：")
        print(f"   {project_dir / 'start_mcp_servers.bat'}")
        print("4. 使用参数管理服务器：")
        print(f"   {project_dir / 'start_mcp_servers.bat'} stop    # 停止服务器")
        print(f"   {project_dir / 'start_mcp_servers.bat'} status  # 检查状态")
        print("5. 在 Cursor 中开始使用！")
        print(f"6. 查看详细说明：{project_dir / 'QUICK_START.md'}")
    else:
        print("❌ 安装过程中出现错误，请检查上述错误信息")


if __name__ == "__main__":
    main()
