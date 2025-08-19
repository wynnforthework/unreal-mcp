#!/usr/bin/env python3
"""
UMG Agent 快速开始脚本

这个脚本帮助你快速在现有UE项目中设置和使用UMG Agent工具。
"""

import os
import sys
import json
import asyncio
import argparse
from pathlib import Path
from typing import Optional

# 添加agents目录到Python路径
sys.path.insert(0, str(Path(__file__).parent))

from orchestrator import AgentOrchestrator, WorkflowConfig
from config import ConfigManager, AgentConfig


class QuickStartSetup:
    """快速开始设置助手"""
    
    def __init__(self):
        self.project_path: Optional[str] = None
        self.config_path: Optional[str] = None
        self.config_manager: Optional[ConfigManager] = None
    
    def setup_interactive(self):
        """交互式设置"""
        print("🚀 UMG Agent 快速开始设置")
        print("=" * 50)
        
        # 获取项目路径
        while not self.project_path:
            project_input = input("请输入你的UE项目路径 (例: D:/MyGame): ").strip()
            if project_input and Path(project_input).exists():
                self.project_path = project_input
                print(f"✅ 项目路径: {self.project_path}")
            else:
                print("❌ 路径不存在，请重新输入")
        
        # 检查项目文件
        uproject_files = list(Path(self.project_path).glob("*.uproject"))
        if not uproject_files:
            print("⚠️  警告: 未找到.uproject文件，请确认这是一个UE项目目录")
        else:
            print(f"✅ 找到项目文件: {uproject_files[0].name}")
        
        # 获取Widget路径
        widget_path = input("Widget蓝图存放路径 [/Game/Widgets]: ").strip()
        if not widget_path:
            widget_path = "/Game/Widgets"
        
        # 获取C++头文件路径（可选）
        cpp_path = input("C++头文件目录 (可选，按Enter跳过): ").strip()
        cpp_paths = [cpp_path] if cpp_path and Path(cpp_path).exists() else []
        
        # 创建配置
        self.create_config(widget_path, cpp_paths)
        
        print("\n✅ 设置完成！")
        print(f"配置文件已保存到: {self.config_path}")
    
    def create_config(self, widget_path: str, cpp_paths: list):
        """创建配置文件"""
        
        config = AgentConfig()
        
        # UE配置
        config.ue.project_path = self.project_path
        config.ue.widget_path = widget_path
        config.ue.cpp_header_paths = cpp_paths
        
        # 日志配置
        log_dir = Path(self.project_path) / "Logs"
        log_dir.mkdir(exist_ok=True)
        config.logging.file_path = str(log_dir / "agent.log")
        
        # 保存配置
        self.config_path = Path(self.project_path) / "agent_config.yaml"
        self.config_manager = ConfigManager()
        self.config_manager.config = config
        self.config_manager.save_to_file(str(self.config_path), "yaml")
    
    def test_connection(self) -> bool:
        """测试UE连接"""
        print("\n🔍 测试UE连接...")
        
        try:
            import socket
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5)
            result = sock.connect_ex(('127.0.0.1', 55557))
            sock.close()
            
            if result == 0:
                print("✅ UE连接正常 (端口55557)")
                return True
            else:
                print("❌ 无法连接到UE (端口55557)")
                print("请确保:")
                print("  1. Unreal Engine已启动")
                print("  2. UnrealMCP插件已加载")
                print("  3. 端口55557未被占用")
                return False
                
        except Exception as e:
            print(f"❌ 连接测试失败: {e}")
            return False


async def generate_sample_ui(project_path: str):
    """生成示例UI"""
    
    print("\n🎨 生成示例UI...")
    
    # 加载配置
    config_path = Path(project_path) / "agent_config.yaml"
    if not config_path.exists():
        print("❌ 配置文件不存在，请先运行设置")
        return
    
    config_manager = ConfigManager(str(config_path))
    agent_config = config_manager.get_config()
    
    # 创建工作流配置
    workflow_config = WorkflowConfig(
        ue_tcp_host=agent_config.ue.tcp_host,
        ue_tcp_port=agent_config.ue.tcp_port,
        cpp_header_path=agent_config.ue.cpp_header_paths[0] if agent_config.ue.cpp_header_paths else None
    )
    
    # 创建编排器
    orchestrator = AgentOrchestrator(workflow_config)
    
    # UI描述
    ui_requests = [
        {
            "name": "主菜单",
            "description": """
            创建一个游戏主菜单界面，包含：
            - 游戏标题在顶部中央，使用大字体
            - "开始游戏"按钮在中央，使用绿色主题
            - "设置"按钮在开始游戏按钮下方
            - "退出游戏"按钮在底部，使用红色主题
            - 整体使用深色背景
            """
        },
        {
            "name": "玩家HUD",
            "description": """
            创建一个玩家HUD界面，包含：
            - 血量条在左上角，红色进度条
            - 魔法值条在血量条下方，蓝色进度条
            - 得分显示在右上角，大号数字
            - 小地图在右下角，正方形图片
            - 准星在屏幕中央，小图标
            """
        }
    ]
    
    # 生成UI
    for ui_request in ui_requests:
        print(f"\n正在生成: {ui_request['name']}")
        
        try:
            result = await orchestrator.execute_workflow(ui_request['description'])
            
            if result.status == "success":
                print(f"✅ {ui_request['name']} 生成成功!")
                
                exec_result = result.execution_result
                print(f"   Widget路径: {exec_result['widget_blueprint_path']}")
                print(f"   组件数量: {len(exec_result['created_components'])}")
                
                # 显示验证结果
                if result.validation_report:
                    validation = result.validation_report
                    print(f"   验证状态: {validation['overall_status']}")
                    
                    if validation['recommendations']:
                        print("   建议:")
                        for rec in validation['recommendations'][:3]:  # 只显示前3个建议
                            print(f"     - {rec}")
            else:
                print(f"❌ {ui_request['name']} 生成失败:")
                for error in result.errors[:3]:  # 只显示前3个错误
                    print(f"   - {error}")
                    
        except Exception as e:
            print(f"❌ 生成 {ui_request['name']} 时出错: {e}")


def create_cpp_template(project_path: str, class_name: str):
    """创建C++模板文件"""
    
    print(f"\n📝 创建C++模板: {class_name}")
    
    # 确定源码目录
    source_dir = Path(project_path) / "Source"
    project_dirs = [d for d in source_dir.iterdir() if d.is_dir() and not d.name.startswith('.')]
    
    if not project_dirs:
        print("❌ 未找到源码目录")
        return
    
    ui_dir = project_dirs[0] / "UI"
    ui_dir.mkdir(exist_ok=True)
    
    # 生成头文件内容
    header_content = f'''#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "{class_name}.generated.h"

UCLASS()
class {project_dirs[0].name.upper()}_API U{class_name} : public UUserWidget
{{
    GENERATED_BODY()

public:
    // Widget绑定示例 - 根据实际生成的Widget调整
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Txt_GameTitle;
    
    UPROPERTY(meta = (BindWidget))
    class UButton* Btn_StartGame;
    
    UPROPERTY(meta = (BindWidget))
    class UButton* Btn_Settings;
    
    UPROPERTY(meta = (BindWidget))
    class UButton* Btn_QuitGame;

protected:
    virtual void NativeConstruct() override;

    // 事件处理函数
    UFUNCTION()
    void OnStartGameClicked();
    
    UFUNCTION()
    void OnSettingsClicked();
    
    UFUNCTION()
    void OnQuitGameClicked();
}};
'''
    
    # 保存头文件
    header_path = ui_dir / f"{class_name}.h"
    header_path.write_text(header_content, encoding='utf-8')
    
    print(f"✅ 头文件已创建: {header_path}")
    print("💡 提示: 请根据实际生成的Widget组件名称调整UPROPERTY绑定")


def main():
    """主函数"""
    
    parser = argparse.ArgumentParser(description="UMG Agent 快速开始工具")
    parser.add_argument("--setup", action="store_true", help="运行交互式设置")
    parser.add_argument("--test", action="store_true", help="测试UE连接")
    parser.add_argument("--generate", action="store_true", help="生成示例UI")
    parser.add_argument("--cpp-template", type=str, help="创建C++模板类")
    parser.add_argument("--project", type=str, help="项目路径")
    
    args = parser.parse_args()
    
    # 如果没有参数，显示帮助
    if not any(vars(args).values()):
        print("🚀 UMG Agent 快速开始工具")
        print("=" * 50)
        print("使用方法:")
        print("  python quick_start.py --setup              # 交互式设置")
        print("  python quick_start.py --test               # 测试连接")
        print("  python quick_start.py --generate           # 生成示例UI")
        print("  python quick_start.py --cpp-template MainMenuWidget  # 创建C++模板")
        print("")
        print("完整流程:")
        print("  1. python quick_start.py --setup")
        print("  2. 启动Unreal Engine并加载你的项目")
        print("  3. python quick_start.py --test")
        print("  4. python quick_start.py --generate")
        return
    
    setup = QuickStartSetup()
    
    # 交互式设置
    if args.setup:
        setup.setup_interactive()
        return
    
    # 确定项目路径
    project_path = args.project
    if not project_path:
        # 尝试从当前目录查找
        current_dir = Path.cwd()
        uproject_files = list(current_dir.glob("*.uproject"))
        if uproject_files:
            project_path = str(current_dir)
        else:
            # 查找配置文件
            config_file = current_dir / "agent_config.yaml"
            if config_file.exists():
                try:
                    import yaml
                    with open(config_file) as f:
                        config_data = yaml.safe_load(f)
                    project_path = config_data.get("ue", {}).get("project_path")
                except:
                    pass
    
    if not project_path:
        print("❌ 无法确定项目路径，请使用 --project 参数或在项目目录中运行")
        return
    
    setup.project_path = project_path
    
    # 测试连接
    if args.test:
        setup.test_connection()
        return
    
    # 生成示例UI
    if args.generate:
        asyncio.run(generate_sample_ui(project_path))
        return
    
    # 创建C++模板
    if args.cpp_template:
        create_cpp_template(project_path, args.cpp_template)
        return


if __name__ == "__main__":
    main()
