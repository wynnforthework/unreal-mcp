#!/usr/bin/env python3
"""
测试 MCP 管理功能
"""

import sys
from pathlib import Path

# 添加当前目录到Python路径
sys.path.insert(0, str(Path(__file__).parent))

from install_manager import InstallManager

def test_mcp_management():
    """测试MCP管理功能"""
    print("🧪 Testing MCP Management Functions")
    print("=" * 50)
    
    # 创建安装管理器
    manager = InstallManager()
    
    # 检查是否有项目
    if not manager.projects:
        print("❌ No projects found. Please add a project first.")
        return
    
    # 获取第一个项目进行测试
    project_id = list(manager.projects.keys())[0]
    project = manager.projects[project_id]
    
    print(f"📁 Testing with project: {project.name}")
    print(f"📍 Project path: {project.path}")
    print(f"🔧 Has MCP tools: {project.has_mcp_tools}")
    print()
    
    if not project.has_mcp_tools:
        print("⚠️  Project doesn't have MCP tools installed. Skipping server tests.")
        return
    
    # 测试详细状态检查
    print("1. Testing detailed MCP status check...")
    try:
        status_info = manager.get_detailed_mcp_status(project_id)
        print(f"✅ Status check result:")
        print(f"   Running: {status_info.get('running', False)}")
        print(f"   Running count: {status_info.get('running_count', 0)}")
        print(f"   Total servers: {status_info.get('total_servers', 0)}")
        print(f"   Method: {status_info.get('method', 'unknown')}")
        
        if 'servers' in status_info:
            print("   Server details:")
            for server in status_info['servers']:
                status_icon = "✅" if server['status'] == 'running' else "❌"
                print(f"     {status_icon} {server['name']}: {server['status']}")
        
        if 'output' in status_info:
            print(f"   Script output: {status_info['output'][:200]}...")
            
    except Exception as e:
        print(f"❌ Status check failed: {e}")
    
    print()
    
    # 测试停止服务器
    print("2. Testing stop MCP servers...")
    try:
        success, message = manager.stop_mcp_servers(project_id)
        print(f"✅ Stop result: {success}")
        print(f"📝 Message: {message}")
    except Exception as e:
        print(f"❌ Stop failed: {e}")
    
    print()
    
    # 测试启动服务器
    print("3. Testing start MCP servers...")
    try:
        success, message = manager.start_mcp_servers(project_id)
        print(f"✅ Start result: {success}")
        print(f"📝 Message: {message}")
    except Exception as e:
        print(f"❌ Start failed: {e}")
    
    print()
    print("🎉 MCP management test completed!")

if __name__ == "__main__":
    test_mcp_management()
