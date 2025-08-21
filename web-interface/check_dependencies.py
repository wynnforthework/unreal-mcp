#!/usr/bin/env python3
"""
检查UMG Agent Web界面的依赖和配置
"""

import sys
import subprocess
from pathlib import Path

def check_python_version():
    """检查Python版本"""
    print("🐍 检查Python版本...")
    version = sys.version_info
    print(f"   Python {version.major}.{version.minor}.{version.micro}")
    
    if version.major < 3 or (version.major == 3 and version.minor < 9):
        print("❌ 需要Python 3.9或更高版本")
        return False
    else:
        print("✅ Python版本符合要求")
        return True

def check_agents_directory():
    """检查agents目录"""
    print("\n📁 检查agents目录...")
    agents_path = Path(__file__).parent.parent / "Python" / "agents"
    
    if not agents_path.exists():
        print(f"❌ Agents目录不存在: {agents_path}")
        return False
    
    print(f"✅ Agents目录存在: {agents_path}")
    
    # 检查关键文件
    required_files = [
        "orchestrator.py",
        "config.py", 
        "base_agent.py",
        "ui_parser_agent.py",
        "design_translator_agent.py",
        "ue_executor_agent.py",
        "binding_validator_agent.py"
    ]
    
    missing_files = []
    for file in required_files:
        if not (agents_path / file).exists():
            missing_files.append(file)
    
    if missing_files:
        print(f"❌ 缺少关键文件: {missing_files}")
        return False
    else:
        print("✅ 所有关键文件都存在")
        return True

def check_dependencies():
    """检查Python依赖"""
    print("\n📦 检查Python依赖...")
    
    required_packages = [
        "flask",
        "flask-socketio", 
        "pydantic",
        "requests",
        "aiohttp",
        "pyyaml"
    ]
    
    missing_packages = []
    
    for package in required_packages:
        try:
            __import__(package.replace("-", "_"))
            print(f"✅ {package}")
        except ImportError:
            print(f"❌ {package}")
            missing_packages.append(package)
    
    if missing_packages:
        print(f"\n📥 需要安装缺少的包:")
        print(f"pip install {' '.join(missing_packages)}")
        return False
    else:
        print("✅ 所有依赖都已安装")
        return True

def test_agents_import():
    """测试agents模块导入"""
    print("\n🔧 测试agents模块导入...")
    
    # 添加agents路径
    agents_path = Path(__file__).parent.parent / "Python" / "agents"
    sys.path.insert(0, str(agents_path))
    
    try:
        from orchestrator import AgentOrchestrator, WorkflowConfig
        print("✅ orchestrator模块导入成功")
        
        from config import ConfigManager, AgentConfig
        print("✅ config模块导入成功")
        
        # 测试创建配置
        config = WorkflowConfig(
            ue_tcp_host="127.0.0.1",
            ue_tcp_port=55557
        )
        print("✅ WorkflowConfig创建成功")
        
        # 测试创建orchestrator
        orchestrator = AgentOrchestrator(config)
        print("✅ AgentOrchestrator创建成功")
        
        return True
        
    except ImportError as e:
        print(f"❌ 导入失败: {e}")
        return False
    except Exception as e:
        print(f"❌ 创建对象失败: {e}")
        return False

def check_ue_connection():
    """检查UE连接"""
    print("\n🎮 检查Unreal Engine连接...")
    
    try:
        import socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        result = sock.connect_ex(('127.0.0.1', 55557))
        sock.close()
        
        if result == 0:
            print("✅ 可以连接到Unreal Engine (端口55557)")
            return True
        else:
            print("⚠️  无法连接到Unreal Engine (端口55557)")
            print("   请确保:")
            print("   1. Unreal Engine已启动")
            print("   2. UnrealMCP插件已加载")
            print("   3. 端口55557未被占用")
            return False
            
    except Exception as e:
        print(f"❌ 连接测试失败: {e}")
        return False

def main():
    """主函数"""
    print("🚀 UMG Agent Web界面依赖检查")
    print("=" * 50)
    
    checks = [
        ("Python版本", check_python_version),
        ("Agents目录", check_agents_directory), 
        ("Python依赖", check_dependencies),
        ("Agents导入", test_agents_import),
        ("UE连接", check_ue_connection)
    ]
    
    results = []
    
    for name, check_func in checks:
        try:
            result = check_func()
            results.append((name, result))
        except Exception as e:
            print(f"❌ {name}检查失败: {e}")
            results.append((name, False))
    
    print("\n" + "=" * 50)
    print("📊 检查结果汇总:")
    
    all_passed = True
    for name, result in results:
        status = "✅" if result else "❌"
        print(f"{status} {name}")
        if not result:
            all_passed = False
    
    print("\n" + "=" * 50)
    if all_passed:
        print("🎉 所有检查都通过！可以启动Web界面:")
        print("python app.py")
    else:
        print("⚠️  有检查项未通过，请先解决上述问题")
        print("\n💡 常见解决方案:")
        print("1. 安装依赖: pip install -r requirements.txt")
        print("2. 安装agents依赖: pip install -r ../Python/agents/requirements.txt") 
        print("3. 确保UE项目已启动并加载UnrealMCP插件")

if __name__ == "__main__":
    main()
