#!/usr/bin/env python3
"""
UMG Agent 项目安装脚本

将UMG Agent工具安装到现有的Unreal Engine项目中
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
    print(f"✅ 插件已复制到: {target_plugin}")
    
    return True


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
    
    requirements_content = """# UMG Agent System Dependencies
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


def create_quick_start_script(project_dir: Path, source_dir: Path) -> bool:
    """复制快速开始脚本到项目目录"""
    
    source_script = source_dir / "Python" / "agents" / "quick_start.py"
    if not source_script.exists():
        print(f"❌ 未找到快速开始脚本: {source_script}")
        return False
    
    target_script = project_dir / "quick_start.py"
    shutil.copy2(source_script, target_script)
    
    print(f"✅ 快速开始脚本已复制到: {target_script}")
    return True


def main():
    """主安装函数"""
    
    print("🚀 UMG Agent 项目安装器")
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
    
    # 2. 复制Agent系统
    print("\n2. 安装Agent系统...")
    if not copy_agent_system(source_dir, project_dir):
        success = False
    
    # 3. 更新项目文件
    print("\n3. 更新项目配置...")
    if not update_uproject_file(project_dir):
        success = False
    
    # 4. 创建依赖文件
    print("\n4. 创建Python依赖文件...")
    if not create_requirements_file(project_dir):
        success = False
    
    # 5. 复制快速开始脚本
    print("\n5. 安装快速开始脚本...")
    if not create_quick_start_script(project_dir, source_dir):
        success = False
    
    # 安装结果
    print("\n" + "=" * 50)
    if success:
        print("🎉 安装完成!")
        print("\n下一步:")
        print("1. 重新启动Unreal Engine并打开你的项目")
        print("2. 确认UnrealMCP插件已加载")
        print("3. 安装Python依赖:")
        print(f"   cd {project_dir / 'Python'}")
        print("   pip install -r requirements.txt")
        print("4. 运行快速开始设置:")
        print(f"   cd {project_dir}")
        print("   python quick_start.py --setup")
        print("5. 测试连接:")
        print("   python quick_start.py --test")
        print("6. 生成示例UI:")
        print("   python quick_start.py --generate")
    else:
        print("❌ 安装过程中出现错误，请检查上述错误信息")


if __name__ == "__main__":
    main()
