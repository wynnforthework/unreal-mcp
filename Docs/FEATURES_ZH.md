# 项目功能总览（中文）

本项目通过 Model Context Protocol（MCP）让 AI 助手（如 Cursor、Windsurf、Claude Desktop 等）以自然语言控制 Unreal Engine 5.6。你可以对 AI 说出意图，系统会以工具调用的形式在 UE 中执行。

## 功能分类

| 类别 | 能力概览 |
|------|-----------|
| Actor 管理 | 创建/删除常见 Actor（立方体、球体、灯光、相机等）；设置位置/旋转/缩放；按名称或模式查找；列出关卡中的 Actor；设置灯光强度/颜色/阴影等；从 Blueprint 类生成实例 |
| Blueprint 开发 | 创建 Blueprint 类（Actor、Pawn 等）；添加/配置组件（静态网格、相机、灯光等）；设置组件属性与物理（模拟、重力、质量、阻尼）；设置类默认值；编译 Blueprint；新增变量（基本类型、结构体、数组、委托、引用）；添加接口；创建自定义事件节点；调用 BlueprintCallable 函数；列出（含继承）组件清单 |
| Blueprint Action 与节点 | 动态发现可用 Blueprint 动作（基于 Pin 类型、类与继承层级）；依据动作名称智能创建节点；检查节点 Pin 需求与类型约束；探索类继承链上的可用能力；构建可视化脚本与连线 |
| UMG/UI | 创建 Widget、布局与交互 UI；添加常见 UMG 组件；支持事件绑定与属性设置；与 Blueprint/节点工具协同构建 UI 逻辑 |
| DataTable | 创建/读取/更新/管理数据表；定义结构体并与表格数据联动；用于角色/物品等游戏数据管理 |
| 项目管理 | 管理内容浏览器文件夹与工程文件夹；Enhanced Input 输入系统（创建 Input Action、Mapping Context、添加按键映射及修饰键）；创建/更新/检查 Unreal 结构体；列出工程与内容文件夹 |
| 编辑器控制 | 视口聚焦/相机控制；基于名称匹配查找场景对象；创建并配置点光/聚光/方向光；调整光照属性；生成带自定义逻辑与组件的 Blueprint 实例 |
| 代理（Agents）系统 | UMG 自动化 4-Agent 工作流：需求解析 → 设计翻译 → UE 执行 → 绑定校验；支持从自然语言描述生成完整 Widget Blueprint，并进行 C++ 绑定校验 |
| UnrealMCP 插件 | 原生 C++ 插件，提供 TCP 服务、命令注册、服务/工厂/工具层，具备校验、日志与错误处理；与编辑器子系统集成 |
| Web 界面（可选） | 轻量 Web 前端（Flask）用于演示/测试部分能力 |

## 组件构成

- 示例工程：`MCPGameProject`（内含 `Plugins/UnrealMCP` 插件）
- 插件：`MCPGameProject/Plugins/UnrealMCP`（C++，提供 MCP 服务与核心能力）
- Python MCP 服务器：`Python/`（按领域拆分的服务器与工具，如 `editor_mcp_server.py`、`blueprint_mcp_server.py` 等）
- Agents 子系统：`Python/agents/`（UMG 端到端自动化工作流）
- Web 界面：`web-interface/`

## 适用场景举例

- 关卡搭建自动化：用自然语言批量生成与布置 Actor、灯光、相机
- 蓝图生产力提升：自动创建类、组件、变量与节点连线
- UI 生产自动化：从自然语言需求直接产出可运行的 Widget Blueprint
- 数据驱动：配合数据表与结构体统一管理游戏数据
- 项目初始化：文件夹/输入系统/结构体的批量化、标准化配置

## 深入阅读

- 核心文档入口：`Docs/README.md`
- 详细指南：
  - `Docs/Blueprint-Tools.md`
  - `Docs/Blueprint-Action-Tools.md`
  - `Docs/Editor-Tools.md`
  - `Docs/Node-Tools.md`
  - `Docs/UMG-Tools.md`
  - `Docs/DataTable-Tools.md`
  - `Docs/Project-Tools.md`
- 插件架构与 API：
  - `MCPGameProject/Plugins/UnrealMCP/Documentation/Architecture_Guide.md`
  - `MCPGameProject/Plugins/UnrealMCP/Documentation/API_Reference.md`
- Agents（UMG 自动化）：`Python/agents/README.md`

> 提示：本项目处于实验阶段，接口与实现可能变更；更多功能细节请以各专题文档为准。


