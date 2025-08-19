# UMG Agent 快速启动指南

这个指南将帮你快速设置和使用UMG Agent的可视化界面。

## 🎯 系统要求

- **Python 3.9+**
- **Unreal Engine 5.6+**
- **UnrealMCP插件** (已包含在项目中)

## 🚀 快速开始 (5分钟设置)

### 1. 安装Python依赖

```bash
# 安装agents系统依赖
cd Python/agents
pip install -r requirements.txt

# 安装Web界面依赖
cd ../../web-interface
pip install -r requirements.txt

# 安装GUI依赖
cd ../Python/gui
pip install -r requirements.txt
```

### 2. 测试系统

```bash
# 回到项目根目录
cd ../../

# 运行系统测试
python test_umg_agent.py
```

如果看到大部分测试通过，说明系统安装正确！

### 3. 选择界面启动

#### 🌐 Web界面 (推荐新手)

```bash
cd web-interface
python check_dependencies.py  # 检查依赖
python app.py                 # 启动服务器
```

然后打开浏览器访问: http://localhost:5000

#### 🖥️ Python GUI

```bash
cd Python/gui
python umg_agent_gui.py
```

#### 🔧 VSCode扩展

```bash
cd vscode-extension
npm install
npm run compile
# 在VSCode中按F5启动调试
```

## 📋 详细设置步骤

### 步骤1: 准备Unreal Engine项目

1. **启动Unreal Engine 5.6+**
2. **打开你的项目** (或创建新项目)
3. **安装UnrealMCP插件**:
   ```bash
   # 复制插件到你的项目
   python install_to_project.py "D:/YourProject"
   ```
4. **重启UE并启用插件**
5. **检查输出日志**，应该看到:
   ```
   UnrealMCPBridge: Server started on 127.0.0.1:55557
   ```

### 步骤2: 配置UMG Agent

创建配置文件 `agent_config.yaml`:

```yaml
ue:
  tcp_host: "127.0.0.1"
  tcp_port: 55557
  project_path: "D:/YourProject"  # 你的项目路径
  widget_path: "/Game/UI/Widgets"

logging:
  level: "INFO"
  file_path: "D:/YourProject/Logs/agent.log"
```

### 步骤3: 测试连接

```bash
# 测试UE连接
python -c "
import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
result = sock.connect_ex(('127.0.0.1', 55557))
sock.close()
print('✅ 连接成功' if result == 0 else '❌ 连接失败')
"
```

## 🎮 使用示例

### 生成主菜单

在任何界面中输入以下描述:

```
创建一个游戏主菜单界面，包含：
- 游戏标题在顶部中央，使用大字体
- "开始游戏"按钮在中央，使用绿色主题
- "设置"按钮在开始游戏按钮下方
- "退出游戏"按钮在底部，使用红色主题
- 整体使用深色背景
```

### 生成玩家HUD

```
创建一个玩家HUD界面，包含：
- 血量条在左上角，红色进度条，显示当前血量/最大血量
- 魔法值条在血量条下方，蓝色进度条
- 得分显示在右上角，大号数字，金色文字
- 小地图在右下角，正方形边框
- 准星在屏幕中央，十字形状
```

## 🔧 故障排除

### 常见问题

#### 1. 导入错误
```
Error importing UMG Agent modules: attempted relative import with no known parent package
```

**解决方案:**
```bash
# 确保在正确目录运行
cd web-interface
python app.py

# 或安装agents为包
cd Python/agents
pip install -e .
```

#### 2. 连接失败
```
❌ Cannot connect to Unreal Engine
```

**解决方案:**
1. 确保UE已启动
2. 确保UnrealMCP插件已加载
3. 检查端口55557是否被占用:
   ```bash
   netstat -an | findstr 55557
   ```

#### 3. 生成失败
```
❌ UI generation failed
```

**解决方案:**
1. 检查UE输出日志中的错误信息
2. 确认项目路径配置正确
3. 确认Widget存储路径存在

#### 4. 依赖缺失
```
ModuleNotFoundError: No module named 'xxx'
```

**解决方案:**
```bash
# 安装所有依赖
pip install -r Python/agents/requirements.txt
pip install -r web-interface/requirements.txt
pip install -r Python/gui/requirements.txt
```

### 调试技巧

#### 启用详细日志
```python
import logging
logging.basicConfig(level=logging.DEBUG)
```

#### 检查agents目录
```bash
ls -la Python/agents/
# 应该看到: orchestrator.py, config.py, base_agent.py 等
```

#### 手动测试导入
```python
import sys
from pathlib import Path
sys.path.insert(0, str(Path("Python/agents")))

from orchestrator import AgentOrchestrator, WorkflowConfig
print("导入成功!")
```

## 🎨 界面对比

| 特性 | Web界面 | Python GUI | VSCode扩展 | UE Slate |
|------|---------|------------|------------|----------|
| 易用性 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |
| 功能完整性 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |
| 安装难度 | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐ |
| 团队协作 | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ | ⭐⭐ |

## 📚 进阶使用

### 批量生成UI
```python
ui_requests = [
    "创建主菜单界面",
    "创建设置菜单界面", 
    "创建暂停菜单界面"
]

for request in ui_requests:
    result = await orchestrator.execute_workflow(request)
    print(f"生成结果: {result.status}")
```

### 自定义配置
```yaml
# 高级配置
ue:
  connection_timeout: 30.0
  command_timeout: 60.0
  max_retries: 5

validation:
  strict_naming: true
  require_cpp_bindings: false
  validate_widget_types: true

llm:
  provider: "openai"
  model: "gpt-4"
  temperature: 0.7
```

### C++绑定验证
```cpp
// 创建对应的C++类
UCLASS()
class UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UButton* Btn_StartGame;
    
    UPROPERTY(meta = (BindWidget))
    class UButton* Btn_Settings;
};
```

## 🎉 成功！

如果你能看到UI在UE中成功生成，恭喜你！UMG Agent已经正常工作了。

现在你可以:
- 🎨 尝试更复杂的UI描述
- 📊 查看生成历史和统计
- ⚙️ 调整配置优化体验
- 👥 与团队分享Web界面

享受可视化的UI开发体验吧！🚀
