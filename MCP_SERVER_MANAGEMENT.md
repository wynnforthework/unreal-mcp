# Unreal MCP Server Management Scripts

## Overview

To solve MCP server process management issues, we provide the following scripts:

- `start_mcp_servers.bat` - Start all MCP servers
- `stop_mcp_servers.bat` - Stop all MCP servers  
- `check_mcp_status.bat` - Check server status

## Problem Solution

### Original Issues
- Python processes remain running in background when closing terminal windows
- Multiple processes appear when restarting (7 → 14 processes)
- Lack of process management mechanism

### Solutions
- Automatically stop existing processes before starting
- Use window titles to identify specific processes
- Provide dedicated stop and status check functions

## Script Features

### 1. start_mcp_servers.bat

**Function:** Start all 7 MCP servers

**Features:**
- Automatically stop existing processes before starting
- Set unique window titles for each server
- Provide usage instructions
- UTF-8 encoding support to prevent garbled text

**Usage:**
```bash
# Start directly
start_mcp_servers.bat

# Start with parameters
start_mcp_servers.bat stop    # Stop all servers
start_mcp_servers.bat status  # Check status
```

### 2. stop_mcp_servers.bat

**Function:** Quickly stop all MCP servers

**Features:**
- Force terminate all MCP related processes
- Include Python and uv processes
- Wait for processes to fully stop
- UTF-8 encoding support

**Usage:**
```bash
stop_mcp_servers.bat
```

### 3. check_mcp_status.bat

**Function:** Detailed check of each server's running status

**Features:**
- Check each of the 7 servers individually
- Display running status statistics
- Provide operation suggestions
- UTF-8 encoding support

**Usage:**
```bash
check_mcp_status.bat
```

## Server List

1. **UMG MCP Server** - UMG/UI tools
2. **Blueprint MCP Server** - Blueprint development tools
3. **Editor MCP Server** - Editor control tools
4. **Node MCP Server** - Node/node tools
5. **DataTable MCP Server** - DataTable tools
6. **Project MCP Server** - Project management tools
7. **Blueprint Action MCP Server** - Blueprint Action tools

## Usage Recommendations

### Daily Workflow
1. **Start servers:** Double-click `start_mcp_servers.bat`
2. **Check status:** Run `check_mcp_status.bat` to confirm all servers are normal
3. **Stop servers:** Run `stop_mcp_servers.bat` or use `start_mcp_servers.bat stop`

### Troubleshooting
- If multiple processes are found, run the stop script first
- Use status check script to confirm process status
- Ensure no other Python processes are occupying ports

### Important Notes
- Closing terminal windows will NOT stop server processes
- Must use dedicated stop scripts
- Recommend regular status checks

## Technical Details

### Process Identification
- Use window title `*MCP*` pattern matching
- Terminate both `python.exe` and `uv.exe` processes
- Use `taskkill /f` to force terminate

### Startup Sequence
- 1-second delay between servers to avoid port conflicts
- Each server has unique window title
- Use `uv run` command to start

### Error Handling
- All `taskkill` commands use `>nul 2>&1` to suppress errors
- Use `timeout` to wait for processes to fully stop
- Provide clear status feedback

### Encoding Support
- Added `chcp 65001` for UTF-8 encoding
- Replaced emoji and Chinese text with English
- Prevents terminal garbled text issues
