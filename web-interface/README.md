# UMG Agent Web Interface

基于Flask和WebSocket的Web界面，为UMG Agent提供浏览器访问的可视化操作界面。

## 功能特性

- 🌐 **Web界面** - 通过浏览器访问，无需安装客户端
- 💬 **实时聊天** - WebSocket实时通信，即时响应
- ⚙️ **可视化配置** - 图形化配置所有参数
- 📊 **生成历史** - 查看和管理UI生成历史
- 📱 **响应式设计** - 支持桌面和移动设备
- 🔄 **实时状态** - 实时显示连接状态和生成进度

## 快速开始

### 1. 安装依赖

```bash
cd web-interface
pip install -r requirements.txt
```

### 2. 启动服务器

```bash
python app.py
```

### 3. 访问界面

打开浏览器访问: http://localhost:5000

## 使用说明

### 配置设置

1. **TCP Host/Port**: UE连接地址和端口
2. **Project Path**: Unreal Engine项目路径
3. **Widget Path**: Widget蓝图存储路径

### UI生成

1. 在聊天框中输入UI描述
2. 点击"Generate"按钮或按Ctrl+Enter
3. 等待生成完成，查看结果

### 历史记录

- 查看所有UI生成历史
- 显示成功/失败状态
- 显示组件数量和时间

## API接口

### REST API

- `GET /api/config` - 获取配置
- `POST /api/config` - 更新配置
- `POST /api/test-connection` - 测试连接
- `GET /api/history` - 获取历史记录
- `DELETE /api/history` - 清空历史记录

### WebSocket事件

- `connect` - 客户端连接
- `generate_ui` - 生成UI请求
- `test_connection` - 测试连接
- `chat_message` - 聊天消息
- `ui_generation_result` - 生成结果

## 部署

### 开发环境

```bash
python app.py
```

### 生产环境

使用Gunicorn部署:

```bash
pip install gunicorn
gunicorn --worker-class eventlet -w 1 --bind 0.0.0.0:5000 app:app
```

使用Docker部署:

```dockerfile
FROM python:3.9-slim

WORKDIR /app
COPY requirements.txt .
RUN pip install -r requirements.txt

COPY . .
EXPOSE 5000

CMD ["gunicorn", "--worker-class", "eventlet", "-w", "1", "--bind", "0.0.0.0:5000", "app:app"]
```

## 配置文件

服务器会自动创建`config.json`配置文件:

```json
{
  "ue_tcp_host": "127.0.0.1",
  "ue_tcp_port": 55557,
  "project_path": "",
  "widget_path": "/Game/Widgets",
  "cpp_header_paths": []
}
```

## 故障排除

### 连接问题

1. 确认Unreal Engine已启动
2. 确认UnrealMCP插件已加载
3. 检查防火墙设置
4. 验证TCP端口是否正确

### 生成失败

1. 检查项目路径配置
2. 确认UMG Agent系统已安装
3. 查看浏览器控制台错误信息
4. 检查服务器日志

### 性能优化

1. 使用生产级WSGI服务器
2. 启用Redis作为消息队列
3. 配置负载均衡
4. 启用HTTPS

## 自定义

### 主题定制

修改`templates/index.html`中的CSS样式:

```css
/* 自定义主题色 */
body {
    background: linear-gradient(135deg, #your-color1 0%, #your-color2 100%);
}
```

### 功能扩展

在`app.py`中添加新的API端点:

```python
@app.route('/api/custom-endpoint', methods=['POST'])
def custom_endpoint():
    # 自定义功能实现
    return jsonify({'status': 'success'})
```

## 许可证

MIT License
