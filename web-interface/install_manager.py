#!/usr/bin/env python3
"""
安装管理器 - 为Web界面提供项目安装功能
"""

import os
import subprocess
import shutil
import json
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import asyncio
from dataclasses import dataclass
from datetime import datetime
import socket
import platform
import time

@dataclass
class ProjectInfo:
    """项目信息"""
    id: str  # 唯一项目ID
    name: str
    path: str
    status: str  # "configured", "installing", "installed", "error"
    last_updated: str
    has_mcp_tools: bool = False
    ue_version: str = ""
    error_message: str = ""

class InstallManager:
    """安装管理器"""
    
    def __init__(self):
        self.projects_config_file = Path("projects_config.json")
        self.projects: Dict[str, ProjectInfo] = {}  # 使用project_id作为key
        self.load_projects()
    
    def generate_project_id(self, project_name: str, project_path: str) -> str:
        """生成唯一的项目ID"""
        import hashlib
        # 使用项目名称和路径的组合生成唯一ID
        id_source = f"{project_name}:{project_path}"
        return hashlib.md5(id_source.encode('utf-8')).hexdigest()[:12]
    
    def is_unreal_editor_running(self) -> bool:
        """检查 Unreal Editor 是否正在运行"""
        try:
            if platform.system() == "Windows":
                result = subprocess.run(
                    ["tasklist", "/FI", "IMAGENAME eq UnrealEditor.exe"],
                    capture_output=True,
                    text=True,
                    check=True
                )
                return "UnrealEditor.exe" in result.stdout
            else:
                # Linux/Mac 系统
                result = subprocess.run(
                    ["pgrep", "-f", "UnrealEditor"],
                    capture_output=True,
                    text=True
                )
                return result.returncode == 0
        except Exception as e:
            print(f"Error checking Unreal Editor status: {e}")
            return False
    
    def close_unreal_editor(self) -> Tuple[bool, str]:
        """关闭 Unreal Editor"""
        try:
            if platform.system() == "Windows":
                result = subprocess.run(
                    ["taskkill", "/F", "/IM", "UnrealEditor.exe"],
                    capture_output=True,
                    text=True
                )
                if result.returncode == 0:
                    return True, "Unreal Editor closed successfully"
                else:
                    return False, "No Unreal Editor process found or failed to close"
            else:
                # Linux/Mac 系统
                result = subprocess.run(
                    ["pkill", "-f", "UnrealEditor"],
                    capture_output=True,
                    text=True
                )
                if result.returncode == 0:
                    return True, "Unreal Editor closed successfully"
                else:
                    return False, "No Unreal Editor process found or failed to close"
        except Exception as e:
            return False, f"Error closing Unreal Editor: {str(e)}"
    
    def open_unreal_editor(self, project_path: str) -> Tuple[bool, str]:
        """打开 Unreal Editor 并加载指定项目"""
        try:
            project_dir = Path(project_path)
            uproject_files = list(project_dir.glob("*.uproject"))
            
            if not uproject_files:
                return False, "No .uproject file found in the project directory"
            
            uproject_file = uproject_files[0]
            
            if platform.system() == "Windows":
                # Windows: 使用 start 命令打开
                subprocess.Popen([
                    "start", "", str(uproject_file)
                ], shell=True)
            else:
                # Linux/Mac: 直接打开
                subprocess.Popen([
                    str(uproject_file)
                ])
            
            return True, f"Unreal Editor opened with project: {uproject_file.name}"
            
        except Exception as e:
            return False, f"Error opening Unreal Editor: {str(e)}"
    
    def load_projects(self):
        """加载项目配置"""
        if self.projects_config_file.exists():
            try:
                with open(self.projects_config_file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    for proj_data in data.get('projects', []):
                        # 兼容旧格式：如果没有ID字段，生成一个
                        if 'id' not in proj_data:
                            proj_data['id'] = self.generate_project_id(
                                proj_data['name'], proj_data['path']
                            )
                        project = ProjectInfo(**proj_data)
                        self.projects[project.id] = project
            except Exception as e:
                print(f"Error loading projects config: {e}")
    
    def save_projects(self):
        """保存项目配置"""
        try:
            data = {
                'projects': [
                    {
                        'id': proj.id,
                        'name': proj.name,
                        'path': proj.path,
                        'status': proj.status,
                        'last_updated': proj.last_updated,
                        'has_mcp_tools': proj.has_mcp_tools,
                        'ue_version': proj.ue_version,
                        'error_message': proj.error_message
                    }
                    for proj in self.projects.values()
                ]
            }
            with open(self.projects_config_file, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
        except Exception as e:
            print(f"Error saving projects config: {e}")
    
    def find_ue_project(self, search_path: str) -> Optional[Path]:
        """查找UE项目文件"""
        search_dir = Path(search_path)
        
        if not search_dir.exists():
            return None
        
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
    
    def get_project_info(self, project_path: str) -> Tuple[bool, Dict]:
        """获取项目信息"""
        project_dir = self.find_ue_project(project_path)
        if not project_dir:
            return False, {"error": "未找到 .uproject 文件"}
        
        # 获取项目名称
        uproject_files = list(project_dir.glob("*.uproject"))
        project_name = uproject_files[0].stem if uproject_files else "Unknown"
        
        # 检查是否已安装 MCP 工具
        has_mcp_tools = self.check_mcp_tools_installed(project_dir)
        
        # 获取 UE 版本（从 .uproject 文件）
        ue_version = self.get_ue_version(project_dir)
        
        return True, {
            "name": project_name,
            "path": str(project_dir),
            "has_mcp_tools": has_mcp_tools,
            "ue_version": ue_version
        }
    
    def check_mcp_tools_installed(self, project_dir: Path) -> bool:
        """检查是否已安装 MCP 工具"""
        # 检查关键文件是否存在
        required_files = [
            project_dir / "Plugins" / "UnrealMCP",
            project_dir / "Python" / "umg_mcp_server.py",
            project_dir / "start_mcp_servers.bat",
            project_dir / ".cursor" / "mcp.json"
        ]
        
        return all(file.exists() for file in required_files)
    
    def get_ue_version(self, project_dir: Path) -> str:
        """获取 UE 版本"""
        try:
            uproject_files = list(project_dir.glob("*.uproject"))
            if uproject_files:
                with open(uproject_files[0], 'r', encoding='utf-8') as f:
                    project_config = json.load(f)
                    return project_config.get("EngineAssociation", "Unknown")
        except:
            pass
        return "Unknown"
    
    def add_project(self, project_path: str) -> Tuple[bool, str]:
        """添加项目"""
        success, info = self.get_project_info(project_path)
        if not success:
            return False, info.get("error", "Unknown error")
        
        # 生成唯一ID
        project_id = self.generate_project_id(info["name"], info["path"])
        
        # 检查是否已存在
        if project_id in self.projects:
            return False, "Project already exists"
        
        project = ProjectInfo(
            id=project_id,
            name=info["name"],
            path=info["path"],
            status="configured",
            last_updated=datetime.now().isoformat(),
            has_mcp_tools=info["has_mcp_tools"],
            ue_version=info["ue_version"]
        )
        
        self.projects[project.id] = project
        self.save_projects()
        
        return True, "Project added successfully"
    
    def remove_project(self, project_id: str) -> Tuple[bool, str]:
        """移除项目"""
        if project_id in self.projects:
            del self.projects[project_id]
            self.save_projects()
            return True, "Project removed successfully"
        return False, "Project not found"
    
    def get_projects_list(self) -> List[Dict]:
        """获取项目列表"""
        projects_list = []
        for project in self.projects.values():
            # 重新检查 MCP 工具状态
            project_dir = Path(project.path)
            if project_dir.exists():
                project.has_mcp_tools = self.check_mcp_tools_installed(project_dir)
            
            projects_list.append({
                "id": project.id,
                "name": project.name,
                "path": project.path,
                "status": project.status,
                "last_updated": project.last_updated,
                "has_mcp_tools": project.has_mcp_tools,
                "ue_version": project.ue_version,
                "error_message": project.error_message
            })
        
        return sorted(projects_list, key=lambda x: x["last_updated"], reverse=True)
    
    async def install_mcp_tools(self, project_id: str) -> Tuple[bool, str]:
        """安装 MCP 工具"""
        if project_id not in self.projects:
            return False, "Project not found"
        
        project = self.projects[project_id]
        project_path = project.path
        project.status = "installing"
        project.error_message = ""
        self.save_projects()
        
        try:
            # 检查并关闭 UE 编辑器
            if self.is_unreal_editor_running():
                print("🔄 Unreal Editor is running, closing it before installation...")
                success, message = self.close_unreal_editor()
                if not success:
                    return False, f"Failed to close Unreal Editor: {message}"
                
                # 等待编辑器完全关闭
                print("⏳ Waiting for Unreal Editor to close...")
                time.sleep(3)
            
            # 获取 install_to_project.py 的路径
            install_script = Path(__file__).parent.parent / "install_to_project.py"
            if not install_script.exists():
                raise FileNotFoundError("install_to_project.py not found")
            
            # 执行安装脚本
            result = await asyncio.create_subprocess_exec(
                "python", str(install_script), project_path,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            
            stdout, stderr = await result.communicate()
            
            if result.returncode == 0:
                # 安装成功，安装 Python 依赖
                python_dir = Path(project_path) / "Python"
                if python_dir.exists():
                    pip_result = await asyncio.create_subprocess_exec(
                        "pip", "install", "-r", str(python_dir / "requirements.txt"),
                        stdout=asyncio.subprocess.PIPE,
                        stderr=asyncio.subprocess.PIPE,
                        cwd=str(python_dir)
                    )
                    
                    await pip_result.communicate()
                
                project.status = "installed"
                project.has_mcp_tools = True
                project.last_updated = datetime.now().isoformat()
                self.save_projects()
                
                return True, "MCP tools installed successfully"
            else:
                error_msg = stderr.decode('utf-8') if stderr else "Installation failed"
                project.status = "error"
                project.error_message = error_msg
                self.save_projects()
                
                return False, error_msg
                
        except Exception as e:
            project.status = "error"
            project.error_message = str(e)
            self.save_projects()
            
            return False, str(e)
    
    def check_python_dependencies(self) -> Tuple[bool, str]:
        """检查 Python 环境"""
        try:
            # 检查 Python 版本
            result = subprocess.run(
                ["python", "--version"],
                capture_output=True,
                text=True,
                check=True
            )
            
            python_version = result.stdout.strip()
            
            # 检查 pip
            pip_result = subprocess.run(
                ["pip", "--version"],
                capture_output=True,
                text=True,
                check=True
            )
            
            return True, f"{python_version}, pip available"
            
        except subprocess.CalledProcessError:
            return False, "Python or pip not found"
        except FileNotFoundError:
            return False, "Python not installed"
    
    def get_installation_status(self, project_id: str) -> Dict:
        """获取安装状态"""
        if project_id not in self.projects:
            return {"status": "not_found"}
        
        project = self.projects[project_id]
        return {
            "status": project.status,
            "has_mcp_tools": project.has_mcp_tools,
            "error_message": project.error_message,
            "last_updated": project.last_updated
        }
    
    def start_mcp_servers(self, project_id: str) -> Tuple[bool, str]:
        """启动项目的 MCP 服务器"""
        print(f"🔍 Looking for project ID: '{project_id}'")
        print(f"📋 Available projects: {list(self.projects.keys())}")
        
        if project_id not in self.projects:
            return False, f"Project not found. Looking for: '{project_id}', Available: {list(self.projects.keys())}"
        
        project = self.projects[project_id]
        project_path = project.path
        if not project.has_mcp_tools:
            return False, "MCP tools not installed in this project"
        
        # 检查并启动 UE 编辑器
        if not self.is_unreal_editor_running():
            print("🔄 Unreal Editor is not running, opening it with the project...")
            success, message = self.open_unreal_editor(project_path)
            if not success:
                return False, f"Failed to open Unreal Editor: {message}"
            
            # 等待编辑器启动
            print("⏳ Waiting for Unreal Editor to start...")
            time.sleep(5)
        else:
            print("✅ Unreal Editor is already running")
        
        project_dir = Path(project_path)
        start_script = project_dir / "start_mcp_servers.bat"
        
        print(f"📂 Project directory: {project_dir}")
        print(f"📄 Start script path: {start_script}")
        print(f"✅ Script exists: {start_script.exists()}")
        
        if not start_script.exists():
            return False, f"start_mcp_servers.bat not found at: {start_script}"
        
        try:
            # 在新的命令提示符窗口中启动批处理脚本
            if platform.system() == "Windows":
                print("🪟 Running on Windows, using CREATE_NEW_CONSOLE")
                # Windows 系统使用 CREATE_NEW_CONSOLE
                process = subprocess.Popen(
                    [str(start_script)],
                    cwd=str(project_dir),
                    shell=True,
                    creationflags=subprocess.CREATE_NEW_CONSOLE
                )
                print(f"🚀 Process started with PID: {process.pid}")
            else:
                print("🐧 Running on non-Windows system")
                # 其他系统的处理方式
                process = subprocess.Popen(
                    [str(start_script)],
                    cwd=str(project_dir),
                    shell=True
                )
                print(f"🚀 Process started with PID: {process.pid}")
            
            return True, f"MCP servers started successfully (PID: {process.pid})"
            
        except Exception as e:
            error_msg = f"Failed to start MCP servers: {str(e)}"
            print(f"💥 Exception: {error_msg}")
            return False, error_msg
    
    def check_mcp_servers_running(self, project_id: str) -> Tuple[bool, List[str]]:
        """检查 MCP 服务器是否运行"""
        if project_id not in self.projects:
            return False, ["Project not found"]
        
        project = self.projects[project_id]
        if not project.has_mcp_tools:
            return False, ["MCP tools not installed"]
        
        # 检查常用的 MCP 服务器端口
        mcp_ports = [
            ("UMG MCP Server", 8001),
            ("Blueprint MCP Server", 8002),
            ("Editor MCP Server", 8003),
            ("Node MCP Server", 8004),
            ("DataTable MCP Server", 8005),
            ("Project MCP Server", 8006),
            ("Actor MCP Server", 8007),
        ]
        
        running_servers = []
        for server_name, port in mcp_ports:
            try:
                import socket
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(1)
                result = sock.connect_ex(("127.0.0.1", port))
                sock.close()
                
                if result == 0:
                    running_servers.append(f"{server_name} (port {port})")
            except:
                pass
        
        return len(running_servers) > 0, running_servers
