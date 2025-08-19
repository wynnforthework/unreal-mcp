# 集成指南：将UMG Agent工具集成到现有项目

## 1. 项目准备

### 检查项目要求
```bash
# 确保项目满足以下要求：
- Unreal Engine 5.6+
- Python插件已启用
- 项目支持UMG (Unreal Motion Graphics)
- 有C++代码支持（可选，用于绑定验证）
```

### 项目结构示例
```
YourGameProject/
├── YourGameProject.uproject
├── Source/
│   └── YourGameProject/
│       ├── UI/                    # 你的UI C++类
│       │   ├── MainMenuWidget.h
│       │   ├── PlayerHUDWidget.h
│       │   └── SettingsWidget.h
│       └── ...
├── Content/
│   ├── Widgets/                   # UMG Widget蓝图目录
│   ├── UI/                        # UI资源
│   └── ...
└── Plugins/                       # 插件目录
    └── UnrealMCP/                 # 将要复制到这里
```

## 2. 安装UnrealMCP插件

### 复制插件文件
```bash
# 从unreal-mcp项目复制插件
cp -r /path/to/unreal-mcp/MCPGameProject/Plugins/UnrealMCP /path/to/YourGameProject/Plugins/

# 或者在Windows上
xcopy "D:\UGit\unreal-mcp\MCPGameProject\Plugins\UnrealMCP" "D:\YourProject\Plugins\UnrealMCP" /E /I
```

### 更新项目文件
在你的 `YourGameProject.uproject` 中添加插件：

```json
{
    "FileVersion": 3,
    "EngineAssociation": "5.6",
    "Category": "",
    "Description": "",
    "Modules": [
        {
            "Name": "YourGameProject",
            "Type": "Runtime",
            "LoadingPhase": "Default"
        }
    ],
    "Plugins": [
        {
            "Name": "UnrealMCP",
            "Enabled": true
        },
        {
            "Name": "PythonScriptPlugin",
            "Enabled": true
        }
    ]
}
```

### 更新Build.cs文件
在你的 `Source/YourGameProject/YourGameProject.Build.cs` 中添加UMG依赖：

```csharp
public class YourGameProject : ModuleRules
{
    public YourGameProject(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore",
            "UMG",           // 添加UMG支持
            "Slate",         // UI框架
            "SlateCore"      // UI核心
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "UnrealMCP"      // 添加MCP插件依赖（可选）
        });
    }
}
```

## 3. 安装Python Agent系统

### 复制Agent代码
```bash
# 复制agents目录到你的项目
cp -r /path/to/unreal-mcp/Python/agents /path/to/YourProject/Python/
```

### 安装Python依赖
```bash
cd /path/to/YourProject/Python
pip install -r requirements.txt

# 或者使用uv（推荐）
uv sync
```

### 创建requirements.txt
```txt
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
```

## 4. 配置Agent系统

### 创建配置文件
在项目根目录创建 `agent_config.yaml`：

```yaml
# agent_config.yaml
ue:
  tcp_host: "127.0.0.1"
  tcp_port: 55557
  project_path: "D:/YourProject"  # 你的项目路径
  widget_path: "/Game/UI/Widgets"  # Widget蓝图存放路径
  cpp_header_paths:
    - "D:/YourProject/Source/YourGameProject/UI/"  # C++头文件路径

figma:
  api_token: ""  # 你的Figma API Token（可选）
  cache_enabled: true

validation:
  strict_naming: true
  require_cpp_bindings: false  # 如果没有C++绑定可设为false
  validate_widget_types: true

logging:
  level: "INFO"
  file_path: "D:/YourProject/Logs/agent.log"
  console_output: true

monitoring:
  enabled: true
  performance_tracking: true
```

### 设置环境变量
```bash
# Windows
set AGENT_UE_PROJECT_PATH=D:\YourProject
set AGENT_UE_WIDGET_PATH=/Game/UI/Widgets
set AGENT_LOG_LEVEL=INFO

# Linux/Mac
export AGENT_UE_PROJECT_PATH=/path/to/YourProject
export AGENT_UE_WIDGET_PATH=/Game/UI/Widgets
export AGENT_LOG_LEVEL=INFO
```

## 5. 启动和测试

### 启动Unreal Engine
```bash
# 启动你的项目
# 确保UnrealMCP插件已加载
# 检查输出日志中是否有 "UnrealMCPBridge: Server started on 127.0.0.1:55557"
```

### 测试连接
创建测试脚本 `test_connection.py`：

```python
import asyncio
import json
import socket

async def test_ue_connection():
    try:
        reader, writer = await asyncio.open_connection('127.0.0.1', 55557)
        
        # 发送ping命令
        command = {"type": "ping", "params": {}}
        writer.write(json.dumps(command).encode() + b'\n')
        await writer.drain()
        
        # 读取响应
        response = await reader.read(1024)
        result = json.loads(response.decode().strip())
        
        print(f"连接成功: {result}")
        
        writer.close()
        await writer.wait_closed()
        
    except Exception as e:
        print(f"连接失败: {e}")

if __name__ == "__main__":
    asyncio.run(test_ue_connection())
```

### 运行第一个UI生成
创建 `generate_ui_example.py`：

```python
import asyncio
from agents.orchestrator import AgentOrchestrator, WorkflowConfig

async def generate_main_menu():
    # 配置
    config = WorkflowConfig(
        ue_tcp_host="127.0.0.1",
        ue_tcp_port=55557,
        cpp_header_path="D:/YourProject/Source/YourGameProject/UI/MainMenuWidget.h"  # 可选
    )
    
    orchestrator = AgentOrchestrator(config)
    
    # 自然语言描述
    request = """
    为我的游戏创建一个主菜单界面，包含：
    - 游戏标题在顶部中央
    - "开始游戏"按钮在中央
    - "设置"按钮在开始游戏按钮下方
    - "退出游戏"按钮在底部
    - 背景使用深色主题
    """
    
    try:
        print("开始生成UI...")
        result = await orchestrator.execute_workflow(request)
        
        if result.status == "success":
            print(f"✅ UI生成成功!")
            print(f"Widget路径: {result.execution_result['widget_blueprint_path']}")
            print(f"创建的组件数量: {len(result.execution_result['created_components'])}")
            
            # 显示验证结果
            if result.validation_report:
                validation = result.validation_report
                print(f"验证状态: {validation['overall_status']}")
                
                if validation['recommendations']:
                    print("建议:")
                    for rec in validation['recommendations']:
                        print(f"  - {rec}")
        else:
            print("❌ UI生成失败:")
            for error in result.errors:
                print(f"  - {error}")
                
    except Exception as e:
        print(f"错误: {e}")

if __name__ == "__main__":
    asyncio.run(generate_main_menu())
```

## 6. 创建C++绑定类（可选）

如果你想要C++绑定验证，创建对应的头文件：

```cpp
// Source/YourGameProject/UI/MainMenuWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MainMenuWidget.generated.h"

UCLASS()
class YOURGAMEPROJECT_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Widget绑定 - 名称需要与生成的Widget组件名称匹配
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

    // 按钮事件处理
    UFUNCTION()
    void OnStartGameClicked();
    
    UFUNCTION()
    void OnSettingsClicked();
    
    UFUNCTION()
    void OnQuitGameClicked();
};
```

## 7. 高级用法

### 批量生成多个UI
```python
# 批量生成UI
ui_requests = [
    "创建玩家HUD，包含血量条和得分显示",
    "创建设置菜单，包含音量滑块和画质选项",
    "创建暂停菜单，包含继续、设置、退出按钮"
]

for i, request in enumerate(ui_requests):
    result = await orchestrator.execute_workflow(request)
    print(f"UI {i+1}: {result.status}")
```

### 从Figma导入设计
```python
from agents.tools.figma_ui_tool import FigmaUITool

async def import_from_figma():
    figma_config = {"figma_token": "your_figma_token"}
    
    async with FigmaUITool(figma_config) as figma:
        ui_spec = await figma.fetch_from_url(
            "https://www.figma.com/file/ABC123/GameUI"
        )
        
        result = await orchestrator.execute_workflow(
            "根据Figma设计创建Widget",
            context={"ui_specification": ui_spec}
        )
```

### 使用MCP服务器
```bash
# 启动MCP服务器
cd /path/to/YourProject/Python
python -m agents.agent_mcp_server
```

然后在Claude Desktop或其他MCP客户端中使用：
```json
{
    "tool": "generate_ui_from_text",
    "arguments": {
        "natural_language_request": "创建一个库存界面，包含物品网格和详情面板",
        "cpp_header_path": "D:/YourProject/Source/YourGameProject/UI/InventoryWidget.h"
    }
}
```

## 8. 故障排除

### 常见问题

1. **连接失败**
   - 检查UE是否启动且UnrealMCP插件已加载
   - 确认端口55557没有被占用
   - 检查防火墙设置

2. **Widget创建失败**
   - 确认Widget路径存在且有写权限
   - 检查UE项目是否支持UMG
   - 查看UE输出日志中的错误信息

3. **C++绑定验证失败**
   - 确认C++头文件路径正确
   - 检查UPROPERTY声明格式
   - 确认Widget组件名称匹配

### 调试技巧
```python
# 启用详细日志
import logging
logging.basicConfig(level=logging.DEBUG)

# 检查工作流进度
progress = orchestrator.get_workflow_progress()
print(f"当前进度: {progress}")

# 获取性能统计
from agents.tools.ue_python_tool import UEPythonTool
async with UEPythonTool() as ue_tool:
    stats = ue_tool.get_performance_stats()
    print(f"性能统计: {stats}")
```

## 9. 生产环境部署

### 配置优化
```yaml
# production_config.yaml
ue:
  connection_timeout: 30.0
  command_timeout: 60.0
  max_retries: 5

logging:
  level: "WARNING"
  file_path: "/var/log/agent.log"
  max_file_size: 50MB
  backup_count: 10

monitoring:
  enabled: true
  metrics_interval: 300.0
  export_metrics: true
  metrics_file: "/var/log/metrics.json"
```

### 服务化部署
```bash
# 创建systemd服务文件
sudo tee /etc/systemd/system/umg-agent.service << EOF
[Unit]
Description=UMG Agent MCP Server
After=network.target

[Service]
Type=simple
User=gamedev
WorkingDirectory=/path/to/YourProject/Python
ExecStart=/usr/bin/python -m agents.agent_mcp_server
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# 启动服务
sudo systemctl enable umg-agent
sudo systemctl start umg-agent
```

这样你就可以将UMG Agent工具完全集成到你的现有项目中，实现自动化的UI生成功能！
