# 按功能分类的使用指南（中文）

本指南汇总各功能的典型用法。你可以直接用自然语言对 AI 助手下达指令；助手会通过 MCP 工具与 UE5.6 协作完成操作。更完整示例请参阅对应专题文档。

## 前置准备

1. 编译并启动示例工程：在仓库根目录执行 `RebuildProject.bat`，然后执行 `LaunchProject.bat`
2. 启动所需 Python MCP 服务器（在 `Python/` 目录）：
   - `python blueprint_mcp_server.py`
   - `python editor_mcp_server.py`
   - `python node_mcp_server.py`
   - `python umg_mcp_server.py`
   - `python datatable_mcp_server.py`
   - `python project_mcp_server.py`
   - （可选）`python agents/agent_mcp_server.py`

> 变更 Python 代码后需重启相应 MCP 进程；变更 C++ 插件后请重新编译并重启工程。

---

## Actor 管理

- 创建与放置 Actor：
  - 「创建一个立方体，位置 (0,0,100)，缩放 2」
  - 「在原点放一个点光，强度 5000，颜色暖色」
  - 「生成一个相机，朝向角色并拉远 800 单位」

- 查询与操作：
  - 「列出关卡中所有包含 Cube 的 Actor 名称」
  - 「把名为 MainLight 的灯光强度改为 8000，启用阴影」
  - 「把 PlayerStart 移动到 (200, -50, 120) 并旋转 (0, 90, 0)」

参考：`Docs/Editor-Tools.md`

---

## Blueprint 开发

- 创建类与组件：
  - 「创建一个基于 Actor 的 Blueprint，命名 BP_MyActor」
  - 「给 BP_MyActor 添加一个 StaticMeshComponent，并设置网格为 Engine/BasicShapes/Cube」
  - 「给 Pawn 类添加 Camera 与 SpringArm，并开启相机控制旋转」

- 变量与默认值：
  - 「在 BP_MyActor 中添加浮点变量 MoveSpeed，默认值 600」
  - 「给组件 Mesh 开启物理模拟，质量 2.5」

- 编译与生成实例：
  - 「编译 BP_MyActor」
  - 「在关卡中放一个 BP_MyActor，位置 (0, 0, 50)」

参考：`Docs/Blueprint-Tools.md`

---

## Blueprint Action 与节点（可视化脚本）

- 动作发现与节点创建：
  - 「在 BP_MyActor 图表中，基于 float 类型列出可用动作」
  - 「创建一个 Add Float + Float 节点，并把变量 A、B 连接到输入」
  - 「为 OnBeginPlay 添加调用 PrintString 的逻辑」

- Pin 需求与连线：
  - 「查看节点 MakeVector 的 Pin 类型与默认值要求」
  - 「把 MakeVector 的输出连到 SetActorLocation 的新位置」

参考：`Docs/Blueprint-Action-Tools.md`、`Docs/Node-Tools.md`

---

## UMG/UI

- 创建与布局：
  - 「创建一个 Widget，包含居中的 StartGame 按钮，下面是 Settings 按钮，上方居中标题」
  - 「添加一个水平布局，内含血条与分数文本，等比分布」

- 事件与绑定：
  - 「将 StartGame 按钮的 OnClicked 绑定到 Blueprint 中的 StartGame 函数」
  - 「将进度条绑定到 PlayerState.Health / PlayerState.MaxHealth」

- Agents 自动化（端到端）：
  - 运行 `python -m agents.agent_mcp_server`
  - 请求示例：
    ```json
    {
      "tool": "generate_ui_from_text",
      "arguments": {
        "natural_language_request": "创建一个带血条和分数的玩家 HUD",
        "cpp_header_path": "D:/Project/Source/PlayerHUD.h"
      }
    }
    ```

参考：`Docs/UMG-Tools.md`、`Python/agents/README.md`

---

## DataTable 与结构体

- 结构体与数据表：
  - 「创建一个结构体 FItemData，包含 Name（String）、Price（Float）、Description（String）」
  - 「创建一个 DataTable 使用 FItemData 作为 RowStruct」
  - 「向 DataTable 添加一行：Id=HealthPotion，Name=Health Potion，Price=50」

- 浏览与更新：
  - 「列出 DataTable 所有行的 Id」
  - 「把 HealthPotion 的 Price 改为 75」

参考：`Docs/DataTable-Tools.md`、`Docs/Project-Tools.md`

---

## 项目管理与 Enhanced Input

- 文件夹与组织：
  - 「在内容浏览器下创建文件夹 UI/HUD、UI/Menus」
  - 「在项目根目录下创建 Logs 与 Intermediate 以外的自定义文件夹」

- Enhanced Input：
  - 「创建数字输入动作 Jump，描述：角色跳跃」
  - 「创建 2D 轴输入动作 Look」
  - 「创建输入映射上下文 Default，并添加键位：W/S/A/D → Move；MouseX/MouseY → Look；Space → Jump」

参考：`Docs/Project-Tools.md`

---

## 编辑器控制与视口

- 视口与查找：
  - 「把视口对准名为 PlayerStart 的 Actor，距离 600」
  - 「按通配符 *Light* 列出匹配的 Actor」

- 灯光与属性：
  - 「创建一个方向光，强度 10 万，颜色偏暖，启用阴影」
  - 「设置 SpotLight 的内外锥角与阴影偏移」

参考：`Docs/Editor-Tools.md`

---

## Web 界面（可选）

- 路径：`web-interface/`
- 启动：
  - 安装依赖：`pip install -r web-interface/requirements.txt`
  - 运行：`python web-interface/app.py`

---

## 常见问题

- 工具不可用/调用失败：检查对应 MCP 服务器是否已启动，或查看 `MCPGameProject\Saved\Logs\MCPGameProject.log`
- C++ 代码改动未生效：先执行 `RebuildProject.bat` 再 `LaunchProject.bat`
- Python 改动未生效：重启对应的 `*_mcp_server.py`

---

## 进一步阅读

- 总览：`Docs/FEATURES_ZH.md`、`Docs/README.md`
- 详细专题：`Docs/*-Tools.md`
- 插件文档：`MCPGameProject/Plugins/UnrealMCP/Documentation/*`


