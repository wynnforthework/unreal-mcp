@echo off
echo 🚀 启动 Unreal MCP 工具集...
cd /d "D:\UGit\unreal-mcp\Python"

echo 启动所有 MCP 服务器...
start "UMG MCP" cmd /k "uv run umg_mcp_server.py"
start "Blueprint MCP" cmd /k "uv run blueprint_mcp_server.py"
start "Editor MCP" cmd /k "uv run editor_mcp_server.py"
start "Node MCP" cmd /k "uv run node_mcp_server.py"
start "DataTable MCP" cmd /k "uv run datatable_mcp_server.py"
start "Project MCP" cmd /k "uv run project_mcp_server.py"
start "Blueprint Action MCP" cmd /k "uv run blueprint_action_mcp_server.py"

echo ✅ 所有 MCP 服务器已启动
echo 现在可以在 AI 助手中使用 Unreal MCP 工具了！
pause
