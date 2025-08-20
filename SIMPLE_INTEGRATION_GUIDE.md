# 最简单的集成方案（中文）

## 一分钟快速集成 Unreal MCP

### 方案概述
这是将 Unreal MCP 完整工具集集成到你现有 UE 项目的最简单方法。

### 前置条件
- 现有的 Unreal Engine 5.6 项目
- Python 3.8+ 已安装
- 网络连接（用于安装 Python 依赖）

---

## 🚀 最简单方式：一键完成

### 超简单方式（推荐）⚡
```bash
# 双击运行（Windows）
ONE_CLICK_INSTALL.bat
```
**就这么简单！脚本会自动：**
1. 提示输入项目路径
2. 复制所有文件和插件
3. 安装 Python 依赖
4. 生成启动脚本和配置

### Web界面方式 🌐
```bash
# 双击启动Web管理界面
START_WEB_INTERFACE.bat
```
然后在浏览器中：
1. 添加你的 UE 项目
2. 点击"一键安装"
3. 完成！

---

## 🔧 手动方式：三步完成

### 步骤 1: 运行安装脚本
```bash
# 在 unreal-mcp 目录中运行
python install_to_project.py "你的UE项目路径"

# 或者交互式运行
python install_to_project.py
# 然后输入项目路径
```

### 步骤 2: 安装依赖
```bash
# 进入你的项目目录下的 Python 文件夹
cd "你的项目/Python"
pip install -r requirements.txt
```

### 步骤 3: 启动服务
```bash
# 双击运行（Windows）
你的项目/start_mcp_servers.bat

# 或者手动运行
cd "你的项目/Python"
python umg_mcp_server.py  # 在新终端窗口
python blueprint_mcp_server.py  # 在新终端窗口
python editor_mcp_server.py  # 在新终端窗口
# ... 其他服务器
```

---

## ✅ 完成！

安装完成后，你的项目将拥有：

### 自动安装的组件
- ✅ **UnrealMCP C++ 插件** - 核心通信插件
- ✅ **7个 MCP 服务器** - 完整工具集
- ✅ **Agent 自动化系统** - UMG 端到端生成
- ✅ **一键启动脚本** - `start_mcp_servers.bat`
- ✅ **MCP 配置文件** - 自动为 Cursor/Claude Desktop/Windsurf 生成
- ✅ **快速开始指南** - `QUICK_START.md`

### 可用功能
- 🎯 **Actor 管理** - 创建、删除、查找 Actor
- 🎯 **Blueprint 开发** - 创建类、添加组件、设置属性
- 🎯 **UI 开发** - UMG Widget 创建与自动化
- 🎯 **节点编程** - Blueprint 可视化脚本
- 🎯 **数据管理** - DataTable 和结构体
- 🎯 **项目管理** - 输入系统、文件夹组织
- 🎯 **编辑器控制** - 视口、场景管理

---

## 🔧 在 Cursor 中使用

### 配置已自动完成
安装脚本已自动创建 `.cursor/mcp.json` 配置文件。

### 立即测试
在 Cursor 中尝试：
```
"创建一个立方体，位置在 (100, 200, 50)"
"创建一个基于 Character 的 Blueprint，命名为 BP_Player"
"列出关卡中所有 Actor"
"创建一个包含按钮和文本的简单 UI"
```

---

## 🔧 在其他 AI 助手中使用

### Claude Desktop
复制 `你的项目/mcp_config.json` 内容到：
`%USERPROFILE%\.config\claude-desktop\mcp.json`

### Windsurf
复制 `你的项目/mcp_config.json` 内容到：
`%USERPROFILE%\.config\windsurf\mcp.json`

---

## 📁 集成后的项目结构

```
你的UE项目/
├── Plugins/
│   └── UnrealMCP/          # C++ 插件
├── Python/
│   ├── agents/             # Agent 系统
│   ├── umg_tools/          # UMG 工具
│   ├── blueprint_tools/    # Blueprint 工具
│   ├── editor_tools/       # Editor 工具
│   ├── node_tools/         # Node 工具
│   ├── datatable_tools/    # DataTable 工具
│   ├── project_tools/      # Project 工具
│   ├── utils/              # 工具库
│   ├── requirements.txt    # Python 依赖
│   └── *_mcp_server.py     # 7个 MCP 服务器
├── .cursor/
│   └── mcp.json           # Cursor 配置
├── mcp_config.json        # 通用 MCP 配置
├── start_mcp_servers.bat  # 一键启动脚本
├── QUICK_START.md         # 详细使用指南
└── 你的项目.uproject       # 已更新插件配置
```

---

## 🎯 使用技巧

### 最佳实践
1. **先启动 UE 项目**，确认插件加载成功
2. **再启动 MCP 服务器**，确保通信正常
3. **在 AI 助手中测试**简单命令，验证连接
4. **逐步尝试复杂功能**，熟悉各种工具

### 常见问题
- **工具调用失败？** 检查 MCP 服务器是否运行
- **UE 无响应？** 查看 `项目/Saved/Logs/项目名.log`
- **Python 错误？** 确认依赖已正确安装

### 性能优化
- 只启动需要的 MCP 服务器
- 使用完毕后关闭不需要的服务器
- 定期重启 UE 项目以释放内存

---

## 🆘 需要帮助？

### 文档资源
- **功能总览**: `Docs/FEATURES_ZH.md`
- **详细使用指南**: `Docs/USAGE_GUIDE_ZH.md`
- **AI 集成指南**: `Docs/AI_INTEGRATION_GUIDE_ZH.md`
- **项目快速开始**: `你的项目/QUICK_START.md`

### 故障排查
1. 查看 UE 日志文件
2. 检查 Python 服务器控制台输出
3. 验证 MCP 配置文件格式
4. 确认网络端口未被占用

---

## 🎉 恭喜！

你现在拥有了一个功能完整的 AI 驱动的 Unreal Engine 开发环境！

**开始创造吧！** 🚀
