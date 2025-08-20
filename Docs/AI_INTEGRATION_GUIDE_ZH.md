# AI 助手集成指南（中文）

## 架构说明

本项目采用 **MCP（Model Context Protocol）** 架构，作为**工具服务器**为 AI 助手提供 Unreal Engine 控制能力。

### 角色分工

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│    用户     │───▶│  AI 助手    │───▶│  MCP 协议   │───▶│ unreal-mcp  │
│             │    │ (Cursor等)  │    │             │    │   工具服务器 │
└─────────────┘    └─────────────┘    └─────────────┘    └─────────────┘
                                                              │
                                                              ▼
                                                    ┌─────────────┐
                                                    │Unreal Engine│
                                                    │    5.6      │
                                                    └─────────────┘
```

- **AI 助手**：理解自然语言，决定调用哪些工具
- **unreal-mcp**：提供 MCP 工具，执行具体 UE 操作
- **MCP 协议**：AI 助手与工具服务器间的通信桥梁

## 为什么不需要在项目中配置 API Key？

### 1. 项目定位
- 本项目是**工具提供方**，不是 AI 使用者
- AI 助手的 API Key 配置在 AI 客户端中
- 项目只需要启动 MCP 服务器，等待 AI 助手连接

### 2. 工作流程示例

**用户操作：**
```
用户对 AI 助手说："创建一个立方体，位置在 (0,0,100)"
```

**AI 助手处理：**
1. 理解用户意图
2. 通过 MCP 调用 `create_actor` 工具
3. 传递参数：`{"actor_type": "cube", "location": [0,0,100]}`

**unreal-mcp 执行：**
1. 接收 MCP 请求
2. 在 UE 中创建立方体
3. 返回结果：`{"success": true, "actor_name": "Cube_1"}`

**AI 助手反馈：**
```
"已成功创建一个立方体，命名为 Cube_1"
```

## 配置步骤

### 1. 启动 unreal-mcp 工具服务器

```bash
# 编译并启动 UE 工程
./RebuildProject.bat
./LaunchProject.bat

# 启动 Python MCP 服务器（在 Python/ 目录）
python blueprint_mcp_server.py
python editor_mcp_server.py
python node_mcp_server.py
python umg_mcp_server.py
python datatable_mcp_server.py
python project_mcp_server.py
python blueprint_action_mcp_server.py
```

### 2. 配置 AI 客户端

#### Cursor 配置
在项目根目录创建 `.cursor/mcp.json`：

```json
{
  "mcpServers": {
    "blueprintMCP": {
      "command": "python",
      "args": ["Python/blueprint_mcp_server.py"]
    },
    "editorMCP": {
      "command": "python", 
      "args": ["Python/editor_mcp_server.py"]
    },
    "umgMCP": {
      "command": "python",
      "args": ["Python/umg_mcp_server.py"]
    },
    "nodeMCP": {
      "command": "python",
      "args": ["Python/node_mcp_server.py"]
    },
    "datatableMCP": {
      "command": "python",
      "args": ["Python/datatable_mcp_server.py"]
    },
    "projectMCP": {
      "command": "python",
      "args": ["Python/project_mcp_server.py"]
    },
    "blueprintActionMCP": {
      "command": "python",
      "args": ["Python/blueprint_action_mcp_server.py"]
    }
  }
}
```

#### Claude Desktop 配置
在 `%USERPROFILE%\.config\claude-desktop\mcp.json` 使用相同配置。

#### Windsurf 配置  
在 `%USERPROFILE%\.config\windsurf\mcp.json` 使用相同配置。

### 3. 在 AI 客户端中配置 API Key

**Cursor：**
- 在设置中配置 OpenAI API Key 或 Anthropic API Key
- 重启 Cursor 使 MCP 配置生效

**Claude Desktop：**
- 在设置中配置 Anthropic API Key
- 重启应用

**Windsurf：**
- 在设置中配置相应的 API Key
- 重启应用

## 使用示例

### 在 Cursor 中测试

1. **启动所有服务**
2. **在 Cursor 中对话：**
   ```
   用户：创建一个基于 Character 的 Blueprint，命名为 BP_Player
   
   AI：我来帮你创建一个基于 Character 的 Blueprint。
   [AI 通过 MCP 调用 create_blueprint 工具]
   
   用户：给这个 Blueprint 添加一个 StaticMeshComponent
   
   AI：我来为 BP_Player 添加一个 StaticMeshComponent。
   [AI 通过 MCP 调用 add_component_to_blueprint 工具]
   ```

### 在 Claude Desktop 中测试

同样的对话流程，Claude 会通过 MCP 协议调用工具。

## 常见问题

### Q: 为什么工具调用失败？
A: 检查：
- UE 工程是否已启动
- Python MCP 服务器是否已启动
- MCP 配置文件路径是否正确
- 查看 `MCPGameProject\Saved\Logs\MCPGameProject.log`

### Q: AI 助手无法连接工具？
A: 检查：
- MCP 配置文件格式是否正确
- 路径是否指向正确的 Python 文件
- 是否重启了 AI 客户端

### Q: 如何调试 MCP 连接？
A: 
- 查看 Python 服务器的控制台输出
- 检查 UE 日志文件
- 确认 TCP 端口 55557 是否被占用

## 总结

- **API Key 配置在 AI 客户端**，不在 unreal-mcp 项目中
- **unreal-mcp 提供工具服务**，等待 AI 助手调用
- **MCP 协议** 作为通信桥梁，实现 AI 与工具的分离
- **用户通过自然语言** 与 AI 交互，AI 自动调用相应工具

这种架构的优势是：
- 工具与 AI 解耦，可以独立开发和维护
- 支持多种 AI 助手（Cursor、Claude Desktop、Windsurf 等）
- 工具可以被多个 AI 助手同时使用
- 便于扩展新的工具和功能
