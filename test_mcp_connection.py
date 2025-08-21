#!/usr/bin/env python3
"""
测试MCP服务器连接
"""

import subprocess
import json
import time
import sys
from pathlib import Path

def test_mcp_server(server_name, server_script):
    """测试单个MCP服务器"""
    print(f"\n🔍 测试 {server_name}...")
    
    try:
        # 启动服务器进程
        process = subprocess.Popen(
            ["uv", "run", server_script],
            cwd=Path("Python"),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        # 等待一下让服务器启动
        time.sleep(2)
        
        # 检查进程是否还在运行
        if process.poll() is None:
            print(f"✅ {server_name} 正在运行 (PID: {process.pid})")
            process.terminate()
            return True
        else:
            stdout, stderr = process.communicate()
            print(f"❌ {server_name} 启动失败")
            print(f"stdout: {stdout}")
            print(f"stderr: {stderr}")
            return False
            
    except Exception as e:
        print(f"❌ 测试 {server_name} 时出错: {e}")
        return False

def main():
    """主测试函数"""
    print("🚀 测试MCP服务器连接...")
    
    # 测试所有MCP服务器
    servers = [
        ("Project MCP", "project_mcp_server.py"),
        ("Blueprint MCP", "blueprint_mcp_server.py"),
        ("Editor MCP", "editor_mcp_server.py"),
        ("UMG MCP", "umg_mcp_server.py"),
        ("Node MCP", "node_mcp_server.py"),
        ("DataTable MCP", "datatable_mcp_server.py"),
        ("Blueprint Action MCP", "blueprint_action_mcp_server.py")
    ]
    
    success_count = 0
    for server_name, server_script in servers:
        if test_mcp_server(server_name, server_script):
            success_count += 1
    
    print(f"\n📊 测试结果: {success_count}/{len(servers)} 个服务器正常")
    
    if success_count == len(servers):
        print("✅ 所有MCP服务器都正常工作！")
        print("\n💡 如果Cursor仍然显示'no tools for prompts'，请尝试：")
        print("1. 重启Cursor")
        print("2. 检查Cursor的MCP配置路径")
        print("3. 确认mcp.json文件格式正确")
    else:
        print("❌ 部分MCP服务器有问题，请检查错误信息")

if __name__ == "__main__":
    main()
