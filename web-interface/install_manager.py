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

@dataclass
class ProjectInfo:
    """项目信息"""
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
        self.projects: Dict[str, ProjectInfo] = {}
        self.load_projects()
    
    def load_projects(self):
        """加载项目配置"""
        if self.projects_config_file.exists():
            try:
                with open(self.projects_config_file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    for proj_data in data.get('projects', []):
                        project = ProjectInfo(**proj_data)
                        self.projects[project.path] = project
            except Exception as e:
                print(f"Error loading projects config: {e}")
    
    def save_projects(self):
        """保存项目配置"""
        try:
            data = {
                'projects': [
                    {
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
        
        project = ProjectInfo(
            name=info["name"],
            path=info["path"],
            status="configured",
            last_updated=datetime.now().isoformat(),
            has_mcp_tools=info["has_mcp_tools"],
            ue_version=info["ue_version"]
        )
        
        self.projects[project.path] = project
        self.save_projects()
        
        return True, "Project added successfully"
    
    def remove_project(self, project_path: str) -> Tuple[bool, str]:
        """移除项目"""
        if project_path in self.projects:
            del self.projects[project_path]
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
                "name": project.name,
                "path": project.path,
                "status": project.status,
                "last_updated": project.last_updated,
                "has_mcp_tools": project.has_mcp_tools,
                "ue_version": project.ue_version,
                "error_message": project.error_message
            })
        
        return sorted(projects_list, key=lambda x: x["last_updated"], reverse=True)
    
    async def install_mcp_tools(self, project_path: str) -> Tuple[bool, str]:
        """安装 MCP 工具"""
        if project_path not in self.projects:
            return False, "Project not found"
        
        project = self.projects[project_path]
        project.status = "installing"
        project.error_message = ""
        self.save_projects()
        
        try:
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
    
    def get_installation_status(self, project_path: str) -> Dict:
        """获取安装状态"""
        if project_path not in self.projects:
            return {"status": "not_found"}
        
        project = self.projects[project_path]
        return {
            "status": project.status,
            "has_mcp_tools": project.has_mcp_tools,
            "error_message": project.error_message,
            "last_updated": project.last_updated
        }
