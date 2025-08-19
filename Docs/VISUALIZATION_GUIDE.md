# UMG Agent 可视化界面指南

你的UMG Agent工具现在有了多种可视化界面选择！每种方案都有其独特的优势，你可以根据需要选择最适合的方案。

## 🎯 方案概览

| 方案 | 优势 | 适用场景 | 技术栈 |
|------|------|----------|--------|
| **VSCode扩展** | 集成开发环境，无缝工作流 | 开发者日常使用 | TypeScript, VSCode API |
| **Python GUI** | 本地应用，响应快速 | 独立使用，离线工作 | Python, tkinter |
| **UE Slate窗口** | 直接集成UE，原生体验 | UE内部使用 | C++, Slate UI |
| **Web界面** | 跨平台，易于分享 | 团队协作，远程访问 | Python, Flask, WebSocket |

## 🚀 快速开始

### 1. VSCode扩展插件

**最推荐的方案** - 如果你使用VSCode开发，这是最佳选择。

```bash
# 安装和构建
cd vscode-extension
npm install
npm run compile

# 在VSCode中按F5启动调试
# 或者打包安装: vsce package
```

**特性:**
- 🎮 可视化聊天界面
- 🔗 自动连接检测
- 📊 生成历史记录
- ⚙️ 可视化设置面板
- 🚀 一键生成UI

**使用方法:**
1. 打开包含UE项目的工作区
2. 点击侧边栏的UMG Agent图标
3. 在聊天界面输入UI描述
4. 查看生成结果和历史记录

### 2. Python GUI工具

**最简单的方案** - 无需额外配置，直接运行。

```bash
# 安装依赖
cd Python/gui
pip install -r requirements.txt

# 启动GUI
python umg_agent_gui.py
```

**特性:**
- 🖥️ 原生桌面应用
- 💬 实时聊天界面
- ⚙️ 可视化配置面板
- 📊 生成历史管理
- 🔄 异步任务处理

**使用方法:**
1. 配置UE项目路径和连接参数
2. 测试连接到Unreal Engine
3. 在聊天框输入UI描述
4. 查看生成进度和结果

### 3. UE Slate编辑器窗口

**最集成的方案** - 直接在UE编辑器中使用。

```cpp
// 在UnrealMCP插件中已包含
// 编译插件后在UE中使用
```

**特性:**
- 🎯 原生UE集成
- 🎨 Slate UI界面
- 📝 实时聊天功能
- 📊 生成历史记录
- ⚙️ 配置管理

**使用方法:**
1. 确保UnrealMCP插件已加载
2. 在UE菜单中打开UMG Agent窗口
3. 配置连接参数
4. 开始生成UI

### 4. Web界面

**最灵活的方案** - 支持远程访问和团队协作。

```bash
# 安装依赖
cd web-interface
pip install -r requirements.txt

# 启动服务器
python app.py

# 访问 http://localhost:5000
```

**特性:**
- 🌐 浏览器访问
- 💬 WebSocket实时通信
- 📱 响应式设计
- 🔄 实时状态更新
- 👥 多用户支持

**使用方法:**
1. 启动Web服务器
2. 浏览器访问界面
3. 配置连接参数
4. 在聊天界面生成UI

## 📋 详细对比

### 功能对比

| 功能 | VSCode扩展 | Python GUI | UE Slate | Web界面 |
|------|------------|------------|----------|---------|
| 聊天界面 | ✅ | ✅ | ✅ | ✅ |
| 配置管理 | ✅ | ✅ | ✅ | ✅ |
| 历史记录 | ✅ | ✅ | ✅ | ✅ |
| 连接测试 | ✅ | ✅ | ✅ | ✅ |
| 实时更新 | ✅ | ✅ | ✅ | ✅ |
| 多用户支持 | ❌ | ❌ | ❌ | ✅ |
| 离线使用 | ✅ | ✅ | ✅ | ❌ |
| 跨平台 | ✅ | ✅ | ❌ | ✅ |

### 性能对比

| 方案 | 启动速度 | 响应速度 | 内存占用 | CPU占用 |
|------|----------|----------|----------|---------|
| VSCode扩展 | 快 | 快 | 低 | 低 |
| Python GUI | 快 | 快 | 中 | 低 |
| UE Slate | 中 | 快 | 低 | 低 |
| Web界面 | 中 | 中 | 中 | 中 |

## 🛠️ 安装和配置

### 通用要求

1. **Unreal Engine 5.6+** 已安装
2. **UnrealMCP插件** 已加载
3. **Python 3.9+** 已安装
4. **UMG Agent系统** 已安装到项目

### 项目结构

```
YourProject/
├── YourProject.uproject
├── Plugins/
│   └── UnrealMCP/           # UE插件
├── Python/
│   ├── agents/              # Agent系统
│   └── gui/                 # Python GUI
├── vscode-extension/        # VSCode扩展
├── web-interface/           # Web界面
└── agent_config.yaml        # 配置文件
```

### 配置示例

```yaml
# agent_config.yaml
ue:
  tcp_host: "127.0.0.1"
  tcp_port: 55557
  project_path: "D:/YourProject"
  widget_path: "/Game/UI/Widgets"
  cpp_header_paths:
    - "D:/YourProject/Source/YourProject/UI/"

logging:
  level: "INFO"
  file_path: "D:/YourProject/Logs/agent.log"
```

## 🎨 使用示例

### 生成主菜单界面

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
- 血量条在左上角，红色进度条
- 魔法值条在血量条下方，蓝色进度条
- 得分显示在右上角，大号数字
- 小地图在右下角，正方形图片
- 准星在屏幕中央，小图标
```

## 🔧 故障排除

### 常见问题

1. **连接失败**
   - 检查UE是否启动
   - 确认UnrealMCP插件已加载
   - 验证端口55557未被占用

2. **生成失败**
   - 检查项目路径配置
   - 确认Python依赖已安装
   - 查看错误日志

3. **界面无响应**
   - 重启应用程序
   - 检查网络连接
   - 清空缓存和历史

### 调试技巧

```python
# 启用详细日志
import logging
logging.basicConfig(level=logging.DEBUG)

# 检查连接状态
import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
result = sock.connect_ex(('127.0.0.1', 55557))
print(f"Connection result: {result}")  # 0表示成功
```

## 🚀 推荐工作流

### 开发者工作流

1. **日常开发**: 使用VSCode扩展
2. **快速测试**: 使用Python GUI
3. **UE内调试**: 使用Slate窗口
4. **团队演示**: 使用Web界面

### 团队协作工作流

1. **设计师**: Web界面进行UI设计
2. **程序员**: VSCode扩展进行开发
3. **测试人员**: Python GUI进行测试
4. **项目经理**: Web界面查看进度

## 📈 未来扩展

### 计划功能

- [ ] 实时预览功能
- [ ] 模板库管理
- [ ] 批量生成工具
- [ ] 版本控制集成
- [ ] 云端同步功能

### 自定义扩展

每个界面都支持自定义扩展：

- **VSCode**: 修改TypeScript代码
- **Python GUI**: 修改tkinter界面
- **UE Slate**: 修改C++代码
- **Web界面**: 修改HTML/CSS/JavaScript

选择最适合你的方案，开始享受可视化的UMG Agent体验吧！🎮✨
