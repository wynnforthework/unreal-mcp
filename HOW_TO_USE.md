# 如何使用 Unreal MCP（中文）

## 概述

本文档提供 Unreal MCP 完整工具集的详细使用方法，从安装到高级功能的完整指南。

---

## 🚀 快速开始

### 方法 1: 一键安装（推荐）
```bash
# Windows 用户
双击运行: ONE_CLICK_INSTALL.bat
# 然后按提示操作
```

### 方法 2: 手动安装
```bash
# 1. 运行安装脚本
python install_to_project.py "你的UE项目路径"

# 2. 安装依赖
cd "你的项目/Python"
pip install -r requirements.txt

# 3. 启动服务
双击运行: 你的项目/start_mcp_servers.bat
```

---

## 🎯 核心功能使用指南

### 1. Actor 管理

**创建 Actor：**
```
"创建一个立方体，位置在 (100, 200, 50)"
"在原点放一个点光，强度 5000，颜色暖色"
"生成一个相机，朝向角色并拉远 800 单位"
```

**查询和操作：**
```
"列出关卡中所有包含 Cube 的 Actor 名称"
"把名为 MainLight 的灯光强度改为 8000，启用阴影"
"把 PlayerStart 移动到 (200, -50, 120) 并旋转 (0, 90, 0)"
```

### 2. Blueprint 开发

**创建类与组件：**
```
"创建一个基于 Actor 的 Blueprint，命名 BP_MyActor"
"给 BP_MyActor 添加一个 StaticMeshComponent，并设置网格为 Engine/BasicShapes/Cube"
"给 Pawn 类添加 Camera 与 SpringArm，并开启相机控制旋转"
```

**变量与属性：**
```
"在 BP_MyActor 中添加浮点变量 MoveSpeed，默认值 600"
"给组件 Mesh 开启物理模拟，质量 2.5"
"编译 BP_MyActor"
```

### 3. UMG/UI 开发

**创建界面：**
```
"创建一个 Widget，包含居中的 StartGame 按钮，下面是 Settings 按钮，上方居中标题"
"添加一个水平布局，内含血条与分数文本，等比分布"
```

**事件绑定：**
```
"将 StartGame 按钮的 OnClicked 绑定到 Blueprint 中的 StartGame 函数"
"将进度条绑定到 PlayerState.Health / PlayerState.MaxHealth"
```

### 4. 节点编程

**动作发现：**
```
"在 BP_MyActor 图表中，基于 float 类型列出可用动作"
"创建一个 Add Float + Float 节点，并把变量 A、B 连接到输入"
```

**逻辑构建：**
```
"为 OnBeginPlay 添加调用 PrintString 的逻辑"
"把 MakeVector 的输出连到 SetActorLocation 的新位置"
```

### 5. 数据管理

**结构体与数据表：**
```
"创建一个结构体 FItemData，包含 Name（String）、Price（Float）、Description（String）"
"创建一个 DataTable 使用 FItemData 作为 RowStruct"
"向 DataTable 添加一行：Id=HealthPotion，Name=Health Potion，Price=50"
```

**数据操作：**
```
"列出 DataTable 所有行的 Id"
"把 HealthPotion 的 Price 改为 75"
```

### 6. 项目管理

**文件夹组织：**
```
"在内容浏览器下创建文件夹 UI/HUD、UI/Menus"
"在项目根目录下创建 Logs 与 Intermediate 以外的自定义文件夹"
```

**Enhanced Input：**
```
"创建数字输入动作 Jump，描述：角色跳跃"
"创建 2D 轴输入动作 Look"
"创建输入映射上下文 Default，并添加键位：W/S/A/D → Move；MouseX/MouseY → Look；Space → Jump"
```

### 7. 编辑器控制

**视口操作：**
```
"把视口对准名为 PlayerStart 的 Actor，距离 600"
"按通配符 *Light* 列出匹配的 Actor"
```

**场景管理：**
```
"创建一个方向光，强度 10 万，颜色偏暖，启用阴影"
"设置 SpotLight 的内外锥角与阴影偏移"
```

---

## 🤖 Agent 自动化系统

### UMG Agent（端到端 UI 生成）

**启动 Agent 服务器：**
```bash
cd Python/agents
python agent_mcp_server.py
```

**使用示例：**
```json
{
  "tool": "generate_ui_from_text",
  "arguments": {
    "natural_language_request": "创建一个带血条和分数的玩家 HUD",
    "cpp_header_path": "D:/Project/Source/PlayerHUD.h"
  }
}
```

**Agent 工作流：**
1. **需求解析** - 将自然语言转换为结构化 UI 定义
2. **设计翻译** - 转换为 UMG 兼容的组件定义
3. **UE 执行** - 在 UE 中创建实际的 Widget Blueprint
4. **绑定验证** - 验证 C++ 绑定的正确性

---

## 🔧 配置与调试

### MCP 配置文件

**Cursor 配置：**
项目根目录 `.cursor/mcp.json`

**Claude Desktop 配置：**
`%USERPROFILE%\.config\claude-desktop\mcp.json`

**Windsurf 配置：**
`%USERPROFILE%\.config\windsurf\mcp.json`

### 服务器管理

**启动所有服务器：**
```bash
# Windows
双击运行: start_mcp_servers.bat

# 手动启动
cd Python
python umg_mcp_server.py
python blueprint_mcp_server.py
python editor_mcp_server.py
python node_mcp_server.py
python datatable_mcp_server.py
python project_mcp_server.py
python blueprint_action_mcp_server.py
```

**检查服务器状态：**
- 查看控制台输出
- 检查端口占用情况
- 验证 UE 插件加载状态

### 故障排查

**常见问题：**

1. **工具调用失败**
   - 检查 MCP 服务器是否运行
   - 验证 UE 项目是否启动
   - 查看 `项目/Saved/Logs/项目名.log`

2. **Python 依赖错误**
   ```bash
   cd Python
   pip install -r requirements.txt --upgrade
   ```

3. **插件加载失败**
   - 重新生成 Visual Studio 项目文件
   - 重新编译 C++ 插件
   - 检查 .uproject 文件中的插件配置

4. **MCP 连接问题**
   - 重启 AI 客户端
   - 检查 MCP 配置文件格式
   - 验证文件路径是否正确

---

## 🎨 高级用法

### 组合使用多个工具

**创建完整的游戏角色：**
```
1. "创建一个基于 Character 的 Blueprint，命名 BP_Player"
2. "给 BP_Player 添加 Camera 和 SpringArm 组件"
3. "创建输入映射上下文 PlayerInput，添加 WASD 移动和鼠标视角"
4. "为 BP_Player 添加移动逻辑的节点"
5. "创建一个显示血量的 UMG Widget"
6. "在关卡中生成 BP_Player 实例"
```

**数据驱动的游戏系统：**
```
1. "创建结构体 FWeaponData，包含伤害、射程、弹药等属性"
2. "创建 DataTable 管理武器数据"
3. "创建武器 Blueprint 类，从 DataTable 读取属性"
4. "创建 UI 显示武器信息"
```

### 自动化工作流

**批量创建资产：**
```
"创建 10 个不同类型的敌人 Blueprint，基于 Pawn 类，分别命名为 BP_Enemy01 到 BP_Enemy10"
```

**场景快速搭建：**
```
"创建一个测试关卡，包含地面、光照、玩家起始点和几个测试用的立方体"
```

---

## 📊 性能优化

### 服务器管理
- 只启动需要的 MCP 服务器
- 定期重启长时间运行的服务器
- 监控内存使用情况

### UE 项目优化
- 定期清理临时文件
- 重新编译 C++ 代码
- 检查插件依赖关系

### AI 助手优化
- 使用具体明确的指令
- 分步执行复杂任务
- 验证每步结果

---

## 🆘 获取帮助

### 文档资源
- **功能总览**: `Docs/FEATURES_ZH.md`
- **使用指南**: `Docs/USAGE_GUIDE_ZH.md`
- **AI 集成指南**: `Docs/AI_INTEGRATION_GUIDE_ZH.md`
- **简单集成方案**: `SIMPLE_INTEGRATION_GUIDE.md`

### 社区支持
- GitHub Issues
- 开发者论坛
- Discord 社区

### 日志文件
- UE 日志: `项目/Saved/Logs/项目名.log`
- Python 服务器控制台输出
- AI 客户端调试信息

---

## 🎯 最佳实践

### 开发流程
1. **规划** - 明确要实现的功能
2. **测试** - 用简单命令验证连接
3. **迭代** - 逐步添加复杂功能
4. **验证** - 测试每个组件的功能

### 命令技巧
- 使用具体的名称和参数
- 分步执行复杂操作
- 善用查询命令了解当前状态
- 结合多个工具实现复杂功能

### 项目管理
- 保持项目结构整洁
- 定期备份重要资产
- 使用版本控制管理代码
- 文档化自定义工作流

---

## 🚀 开始创造！

现在你已经掌握了 Unreal MCP 的完整使用方法。开始用自然语言控制 Unreal Engine，创造你的游戏世界吧！

记住：**任何你能用语言描述的 UE 操作，都可以通过 AI 助手来实现！**
