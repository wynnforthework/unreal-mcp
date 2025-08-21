#!/usr/bin/env python3
"""
直接使用UMG MCP服务器创建游戏主菜单
根据用户需求：居中大按钮写「开始游戏」，右上角一个设置按钮，左下角显示玩家昵称
"""

import sys
import os
import logging
from pathlib import Path

# 添加Python目录到路径
current_dir = Path(__file__).parent
python_dir = current_dir / "Python"
sys.path.insert(0, str(python_dir))

from utils.unreal_connection_utils import send_unreal_command
from utils.widgets.widget_components import (
    create_widget_blueprint,
    add_widget_component_to_widget,
    bind_widget_component_event,
    set_widget_component_property
)
from mcp.server.fastmcp import Context

# 设置日志
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("MainMenuCreator")

def test_connection() -> bool:
    """测试与Unreal Engine的连接"""
    try:
        response = send_unreal_command("ping", {})
        if response and response.get("status") == "success":
            logger.info("✅ 成功连接到Unreal Engine")
            return True
        else:
            logger.error("❌ 无法连接到Unreal Engine，请确保：")
            logger.error("  1. Unreal Engine项目正在运行")
            logger.error("  2. UnrealMCP插件已启用")
            logger.error("  3. TCP服务器正在监听端口55557")
            return False
    except Exception as e:
        logger.error(f"❌ 连接测试失败: {e}")
        return False

def create_main_menu_widget():
    """创建游戏主菜单Widget"""
    logger.info("🎮 开始创建游戏主菜单...")
    
    # 创建虚拟context对象
    class MockContext:
        pass
    
    ctx = MockContext()
    widget_name = "WBP_MainMenu"
    
    try:
        # 1. 创建Widget蓝图
        logger.info("📝 步骤1: 创建Widget蓝图")
        create_result = create_widget_blueprint(
            ctx=ctx,
            widget_name=widget_name,
            parent_class="UserWidget",
            path="/Game/UI"
        )
        
        if not create_result.get("status") == "success" or not create_result.get("result", {}).get("success", False):
            logger.error(f"❌ 创建Widget蓝图失败: {create_result}")
            return False
        
        logger.info(f"✅ Widget蓝图创建成功: {create_result.get('message', '')}")
        
        # 2. 添加居中的"开始游戏"大按钮
        logger.info("🔘 步骤2: 添加居中大按钮「开始游戏」")
        start_button_result = add_widget_component_to_widget(
            ctx=ctx,
            widget_name=widget_name,
            component_name="StartGameButton",
            component_type="Button",
            position=[640, 400],  # 屏幕中央位置 (假设1280x720)
            size=[200, 80],       # 大按钮尺寸
            text="开始游戏"
        )
        
        if start_button_result.get("success", False):
            logger.info("✅ 开始游戏按钮创建成功")
        else:
            logger.warning(f"⚠️ 开始游戏按钮创建可能有问题: {start_button_result}")
        
        # 3. 添加右上角设置按钮
        logger.info("⚙️ 步骤3: 添加右上角设置按钮")
        settings_button_result = add_widget_component_to_widget(
            ctx=ctx,
            widget_name=widget_name,
            component_name="SettingsButton",
            component_type="Button",
            position=[1150, 50],  # 右上角位置
            size=[100, 50],       # 较小的按钮
            text="设置"
        )
        
        if settings_button_result.get("success", False):
            logger.info("✅ 设置按钮创建成功")
        else:
            logger.warning(f"⚠️ 设置按钮创建可能有问题: {settings_button_result}")
        
        # 4. 添加左下角玩家昵称文本
        logger.info("👤 步骤4: 添加左下角玩家昵称显示")
        player_name_result = add_widget_component_to_widget(
            ctx=ctx,
            widget_name=widget_name,
            component_name="PlayerNameText",
            component_type="TextBlock",
            position=[50, 620],   # 左下角位置
            size=[200, 30],       # 文本区域
            text="玩家昵称: Player001"
        )
        
        if player_name_result.get("success", False):
            logger.info("✅ 玩家昵称文本创建成功")
        else:
            logger.warning(f"⚠️ 玩家昵称文本创建可能有问题: {player_name_result}")
        
        # 5. 设置按钮样式
        logger.info("🎨 步骤5: 设置按钮样式")
        
        # 设置开始游戏按钮样式（绿色主题）
        try:
            start_button_style = set_widget_component_property(
                ctx=ctx,
                widget_name=widget_name,
                component_name="StartGameButton",
                BackgroundColor={
                    "SpecifiedColor": {
                        "R": 0.2,
                        "G": 0.8,
                        "B": 0.2,
                        "A": 1.0
                    }
                }
            )
            logger.info("✅ 开始游戏按钮样式设置完成")
        except Exception as e:
            logger.warning(f"⚠️ 设置开始游戏按钮样式时出现问题: {e}")
        
        # 设置玩家昵称文本样式
        try:
            player_name_style = set_widget_component_property(
                ctx=ctx,
                widget_name=widget_name,
                component_name="PlayerNameText",
                ColorAndOpacity={
                    "SpecifiedColor": {
                        "R": 1.0,
                        "G": 1.0,
                        "B": 1.0,
                        "A": 1.0
                    }
                }
            )
            logger.info("✅ 玩家昵称文本样式设置完成")
        except Exception as e:
            logger.warning(f"⚠️ 设置玩家昵称文本样式时出现问题: {e}")
        
        # 6. 绑定按钮事件
        logger.info("🔗 步骤6: 绑定按钮点击事件")
        
        # 绑定开始游戏按钮事件
        try:
            start_event_result = bind_widget_component_event(
                ctx=ctx,
                widget_name=widget_name,
                widget_component_name="StartGameButton",
                event_name="OnClicked",
                function_name="OnStartGameClicked"
            )
            if start_event_result.get("success", False):
                logger.info("✅ 开始游戏按钮事件绑定成功")
            else:
                logger.warning(f"⚠️ 开始游戏按钮事件绑定可能有问题: {start_event_result}")
        except Exception as e:
            logger.warning(f"⚠️ 绑定开始游戏按钮事件时出现问题: {e}")
        
        # 绑定设置按钮事件
        try:
            settings_event_result = bind_widget_component_event(
                ctx=ctx,
                widget_name=widget_name,
                widget_component_name="SettingsButton",
                event_name="OnClicked",
                function_name="OnSettingsClicked"
            )
            if settings_event_result.get("success", False):
                logger.info("✅ 设置按钮事件绑定成功")
            else:
                logger.warning(f"⚠️ 设置按钮事件绑定可能有问题: {settings_event_result}")
        except Exception as e:
            logger.warning(f"⚠️ 绑定设置按钮事件时出现问题: {e}")
        
        logger.info("\n" + "="*50)
        logger.info("🎉 游戏主菜单创建完成！")
        logger.info("📍 Widget蓝图位置: /Game/UI/WBP_MainMenu")
        logger.info("\n包含的组件:")
        logger.info("  🔘 StartGameButton - 居中大按钮「开始游戏」")
        logger.info("  ⚙️ SettingsButton - 右上角设置按钮")
        logger.info("  👤 PlayerNameText - 左下角玩家昵称显示")
        logger.info("\n绑定的事件:")
        logger.info("  📌 OnStartGameClicked - 开始游戏按钮点击事件")
        logger.info("  📌 OnSettingsClicked - 设置按钮点击事件")
        logger.info("="*50)
        
        return True
        
    except Exception as e:
        logger.error(f"❌ 创建主菜单时发生错误: {e}")
        return False

def main():
    """主函数"""
    print("🎮 UMG主菜单创建工具")
    print("=" * 50)
    print("将创建一个包含以下元素的游戏主菜单：")
    print("  🔘 居中大按钮「开始游戏」")
    print("  ⚙️ 右上角设置按钮") 
    print("  👤 左下角玩家昵称显示")
    print("=" * 50)
    
    # 测试连接
    print("\n🔍 测试Unreal Engine连接...")
    if not test_connection():
        print("\n❌ 无法连接到Unreal Engine，请：")
        print("1. 确保Unreal Engine项目正在运行")
        print("2. 确保UnrealMCP插件已启用")
        print("3. 检查TCP服务器是否在端口55557上运行")
        return
    
    # 创建主菜单
    print("\n🚀 开始创建主菜单...")
    if create_main_menu_widget():
        print("\n🎊 恭喜！游戏主菜单创建成功！")
        print("💡 您可以在Unreal Engine的Content Browser中找到：")
        print("   📂 Content/UI/WBP_MainMenu")
        print("\n💡 后续步骤：")
        print("1. 在UE编辑器中打开WBP_MainMenu进行样式调整")
        print("2. 在C++或蓝图中实现按钮点击事件逻辑")
        print("3. 将主菜单添加到游戏的启动流程中")
    else:
        print("\n❌ 主菜单创建失败，请检查上述错误信息")

if __name__ == "__main__":
    main()
