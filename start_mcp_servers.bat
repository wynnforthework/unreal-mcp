@echo off
echo 🚀 启动 Unreal MCP 工具集...
cd /d "D:\UGit\unreal-mcp\Python"

echo 启动所有 MCP 服务器...
start "UMG MCP" cmd /k "python umg_mcp_server.py"
start "Blueprint MCP" cmd /k "python blueprint_mcp_server.py"
start "Editor MCP" cmd /k "python editor_mcp_server.py"
start "Node MCP" cmd /k "python node_mcp_server.py"
start "DataTable MCP" cmd /k "python datatable_mcp_server.py"
start "Project MCP" cmd /k "python project_mcp_server.py"
start "Blueprint Action MCP" cmd /k "python blueprint_action_mcp_server.py"

echo ✅ 所有 MCP 服务器已启动
echo 现在可以在 AI 助手中使用 Unreal MCP 工具了！
pause
