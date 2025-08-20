# Enhanced Unreal MCP Web Interface

一个功能强大的 Web 界面，提供项目管理和一键安装 Unreal MCP 工具集的完整解决方案。

## ✨ 新功能特性

### 🎯 项目管理
- **可视化项目浏览器** - 从一个界面管理多个 UE 项目
- **一键安装** - 单击即可安装 Unreal MCP 工具集
- **安装状态跟踪** - 实时进度和状态更新
- **项目切换** - 无缝切换不同项目
- **系统健康监控** - 检查 Python、Agents 和 UE 连接状态

### 🤖 UMG Agent 集成
- **自然语言 UI 生成** - 描述 UI 即可获得 Widget Blueprint
- **实时聊天界面** - 交互式 UI 生成对话
- **可视化反馈** - 实时连接状态和生成进度
- **配置管理** - 轻松设置 UE 连接参数
- **生成历史** - 跟踪和回顾之前的 UI 生成请求

### 🌐 现代 Web 体验
- **响应式设计** - 支持桌面和移动设备
- **实时更新** - 基于 WebSocket 的实时更新
- **直观界面** - 清洁、现代的界面设计

## 🚀 快速开始

### 方式 1: 增强版界面（推荐）

```bash
# 1. 进入 web-interface 目录
cd web-interface

# 2. 安装依赖
pip install -r requirements.txt

# 3. 启动增强版服务器
python app_enhanced.py

# 4. 打开浏览器
# 访问 http://localhost:5000
```

### 方式 2: 原版界面

```bash
# 启动原版服务器
python app.py

# 访问 http://localhost:5000
```

## 📋 使用指南

### 1. 项目管理

#### 添加新项目
1. 点击主页面的 "Add New Project"
2. 输入 UE 项目文件夹路径
3. 系统自动检测项目信息
4. 点击 "Add Project" 保存

#### 安装 MCP 工具
1. 在项目列表中找到您的项目
2. 点击 "Install MCP" 按钮
3. 等待安装完成（自动安装依赖）
4. 完成后状态更新为 "Installed"

#### 选择项目
1. 找到状态为 "Installed" 的项目
2. 点击 "Select" 设为活动项目
3. 将跳转到聊天界面

### 2. UMG Agent 聊天界面

#### 基本使用
1. 确保已选择项目
2. 用自然语言描述要创建的 UI
3. 按 Ctrl+Enter 或点击 "Generate"
4. 观看实时进度和结果

#### 对话示例

**简单 UI：**
```
用户：创建一个包含开始游戏和设置按钮的主菜单

Agent：✅ UI 生成成功！
📁 Widget 路径：/Game/Widgets/WBP_MainMenu
🔧 创建组件：3
🏗️ 项目：MyGameProject
```

**复杂 UI：**
```
用户：创建一个玩家 HUD，包含血条、弹药计数器和小地图

Agent：✅ UI 生成成功！
📁 Widget 路径：/Game/Widgets/WBP_PlayerHUD
🔧 创建组件：6
🏗️ 项目：MyGameProject

💡 建议：
• 将血条绑定到玩家生命值属性
• 添加伤害指示器动画
• 考虑小地图更新优化
```

## ⚙️ 配置管理

### 系统配置

Web 界面自动管理：
- **项目配置**：存储在 `projects_config.json`
- **应用设置**：存储在 `config.json`
- **安装状态**：MCP 工具安装的实时跟踪

### 手动配置

您仍可手动配置：
- **TCP 主机/端口**：UE 连接（默认：127.0.0.1:55557）
- **Widget 路径**：UMG widget 位置（默认：/Game/Widgets）
- **项目设置**：通过 Web 界面

## 🔧 API 参考

### 项目管理 API

- `GET /api/projects` - 列出所有项目
- `POST /api/projects` - 添加新项目
- `DELETE /api/projects/<path>` - 移除项目
- `POST /api/projects/<path>/install` - 安装 MCP 工具
- `GET /api/projects/<path>/status` - 获取安装状态
- `POST /api/projects/<path>/select` - 选择活动项目

### 系统 API

- `GET /api/system/check` - 检查系统健康
- `GET /api/config` - 获取当前配置
- `POST /api/config` - 更新配置
- `POST /api/test-connection` - 测试 UE 连接

### UMG Agent API

- `GET /api/history` - 获取生成历史
- `DELETE /api/history` - 清空生成历史

### WebSocket 事件

- `installation_result` - 安装进度更新
- `generate_ui` - UI 生成请求
- `ui_generation_result` - 生成结果
- `connection_status` - UE 连接状态

## 📁 文件结构

```
web-interface/
├── app_enhanced.py         # 增强版 Flask 应用
├── app.py                  # 原版 Flask 应用
├── install_manager.py      # 项目安装管理器
├── templates/
│   ├── index_enhanced.html # 增强版项目管理界面
│   ├── chat.html          # 聊天界面
│   └── index.html         # 原版界面
├── requirements.txt        # Python 依赖
├── README.md              # 原版文档
├── README_ENHANCED.md     # 本文档
├── projects_config.json   # 项目配置（自动生成）
└── config.json           # 应用配置（自动生成）
```

## 🔍 故障排除

### 安装问题

1. **"Python 不可用"**
   - 从 python.org 安装 Python 3.8+
   - 确保 Python 在系统 PATH 中
   - 重启命令提示符/终端

2. **"安装失败"**
   - 检查项目路径是否正确
   - 确保 .uproject 文件存在
   - 检查写入权限
   - 查看安装日志

### 连接问题

1. **"无法连接到 Unreal Engine"**
   - 先启动您的 UE 项目
   - 确保 UnrealMCP 插件已加载
   - 检查 TCP 端口（默认：55557）未被阻止

2. **"UMG Agents 不可用"**
   - 先使用 Web 界面安装 MCP 工具
   - 检查 Python 依赖已安装
   - 验证 agents 目录存在

### Web 界面问题

1. **"项目未加载"**
   - 检查 `projects_config.json` 权限
   - 刷新浏览器页面
   - 清除浏览器缓存

2. **"安装卡住"**
   - 在任务管理器中检查 Python 进程
   - 重启 Web 界面
   - 尝试手动安装

## 🎯 高级功能

### 批量操作
- 向多个项目安装 MCP 工具
- 导出/导入项目配置
- 批量项目状态更新

### 开发模式
```bash
# 启用调试运行
python app_enhanced.py
```

### 自定义工作流
- 与 CI/CD 管道集成
- 自动化项目设置
- 自定义 UI 生成模板

## 🌐 浏览器兼容性

- Chrome 80+ ✅
- Firefox 76+ ✅
- Safari 13+ ✅
- Edge 80+ ✅

需要 WebSocket 和现代 JavaScript 功能支持。

## 🔒 安全说明

- 仅用于本地开发
- 不要在没有身份验证的情况下暴露给公共网络
- 项目路径和配置存储在本地
- WebSocket 连接仅使用本地主机

## 🤝 贡献

要为 Web 界面做贡献：
1. Fork 仓库
2. 创建功能分支
3. 使用多个 UE 项目进行测试
4. 提交带有描述的 pull request

## 📄 许可证

此增强版 Web 界面是 Unreal MCP 系统的一部分，遵循相同的许可条款。
