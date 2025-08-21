# MCP 服务器管理功能

## 概述

Web界面现在支持完整的MCP服务器管理功能，包括启动、停止和状态检查。

## 新增功能

### 1. 停止MCP服务器
- **功能**: 安全地停止所有运行中的MCP服务器
- **按钮**: 红色停止按钮 (🛑)
- **API**: `POST /api/projects/{project_id}/stop-servers`
- **实现**: 调用项目的 `stop_mcp_servers.bat` 脚本

### 2. 详细状态检查
- **功能**: 获取每个MCP服务器的详细运行状态
- **按钮**: 灰色详细状态按钮 (📋)
- **API**: `GET /api/projects/{project_id}/check-mcp-status`
- **实现**: 调用项目的 `check_mcp_status.bat` 脚本

## 界面更新

### 项目卡片按钮
每个已安装MCP工具的项目现在显示以下按钮：

1. **选择项目** (蓝色) - 选择当前工作项目
2. **启动服务器** (橙色) - 启动所有MCP服务器
3. **停止服务器** (红色) - 停止所有MCP服务器 ⭐ **新增**
4. **检查状态** (蓝色) - 快速状态检查
5. **详细状态** (灰色) - 详细状态报告 ⭐ **新增**
6. **更新MCP** (绿色) - 重新安装/更新MCP工具
7. **打开文件夹** (灰色) - 打开项目文件夹
8. **移除项目** (红色) - 从列表中移除项目

### 状态显示
- 实时显示每个服务器的运行状态
- 显示运行中的服务器数量
- 提供详细的服务器列表

## 技术实现

### 后端API

#### 停止服务器
```python
@app.route('/api/projects/<project_id>/stop-servers', methods=['POST'])
def stop_mcp_servers(project_id):
    """停止项目的 MCP 服务器"""
    # 调用 install_manager.stop_mcp_servers()
```

#### 详细状态检查
```python
@app.route('/api/projects/<project_id>/check-mcp-status')
def check_mcp_status(project_id):
    """检查项目的 MCP 服务器详细状态"""
    # 调用 install_manager.get_detailed_mcp_status()
```

### 安装管理器方法

#### stop_mcp_servers()
- 检查项目是否存在
- 验证MCP工具是否已安装
- 执行 `stop_mcp_servers.bat` 脚本
- 返回操作结果

#### get_detailed_mcp_status()
- 优先使用 `check_mcp_status.bat` 脚本
- 解析脚本输出获取详细状态
- 回退到端口检查方法
- 返回结构化的状态信息

### 前端实现

#### JavaScript函数
- `stopMCPServers(projectId)` - 停止服务器
- `checkDetailedMCPStatus(projectId)` - 详细状态检查

#### 国际化支持
- 新增翻译键：`stop_servers`, `detailed_status`, `servers_stopped`, `stopping_servers`
- 支持中英文界面

## 使用流程

### 1. 启动服务器
1. 点击项目的"启动服务器"按钮
2. 系统自动打开Unreal Editor（如果未运行）
3. 启动所有7个MCP服务器
4. 显示成功通知

### 2. 检查状态
1. 点击"检查状态"按钮进行快速检查
2. 或点击"详细状态"按钮获取完整报告
3. 查看每个服务器的运行状态

### 3. 停止服务器
1. 点击"停止服务器"按钮
2. 系统安全地停止所有MCP服务器
3. 显示停止成功通知

## 错误处理

### 常见错误
- **项目未找到**: 项目ID无效或项目已被删除
- **MCP工具未安装**: 项目尚未安装MCP工具
- **脚本不存在**: 管理脚本文件丢失
- **执行超时**: 脚本执行时间过长

### 错误恢复
- 自动重试机制
- 详细的错误消息
- 回退到备用检查方法

## 测试

### 运行测试脚本
```bash
cd web-interface
python test_mcp_management.py
```

### 测试内容
1. 详细状态检查功能
2. 停止服务器功能
3. 启动服务器功能
4. 错误处理机制

## 注意事项

### 系统要求
- Windows系统（主要支持）
- Python环境
- 已安装MCP工具的项目

### 安全考虑
- 停止操作会强制终止所有相关进程
- 建议在停止前保存工作
- 停止后需要重新启动服务器才能使用

### 性能优化
- 状态检查有30秒超时限制
- 使用异步操作避免界面阻塞
- 缓存状态信息减少重复检查

## 未来改进

### 计划功能
- 实时状态监控
- 自动重启失败的服务器
- 服务器日志查看
- 批量操作支持

### 技术优化
- WebSocket实时状态更新
- 更精确的进程管理
- 跨平台支持改进
