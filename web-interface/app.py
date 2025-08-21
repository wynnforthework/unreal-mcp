#!/usr/bin/env python3
"""
UMG Agent Web Interface
基于Flask的Web界面，提供浏览器访问的UMG Agent功能
"""

from flask import Flask, render_template, request, jsonify, session
from flask_socketio import SocketIO, emit
import asyncio
import json
import sys
import uuid
from pathlib import Path
from datetime import datetime
from typing import Dict, Any, List
import threading
import queue

# 添加agents目录到Python路径
agents_path = Path(__file__).parent.parent / "Python" / "agents"
sys.path.insert(0, str(agents_path))

# 检查agents目录是否存在
if not agents_path.exists():
    print(f"❌ Agents directory not found: {agents_path}")
    print("Please ensure UMG Agent system is properly installed.")
    print("Run the following command to install:")
    print("python install_to_project.py <your-project-path>")
    sys.exit(1)

print(f"📁 Agents path: {agents_path}")
print("📋 Available agent files:")
for file in agents_path.glob("*.py"):
    print(f"  - {file.name}")

try:
    # 导入真正的agents系统
    from orchestrator import AgentOrchestrator, WorkflowConfig
    from config import ConfigManager, AgentConfig
    print("✅ UMG Agent modules imported successfully")
except ImportError as e:
    print(f"❌ Error importing UMG Agent modules: {e}")
    print("This might be due to missing dependencies. Please install:")
    print("pip install -r ../Python/agents/requirements.txt")
    sys.exit(1)

app = Flask(__name__)
app.config['SECRET_KEY'] = 'umg-agent-secret-key'
socketio = SocketIO(app, cors_allowed_origins="*")

# 全局状态
class AppState:
    def __init__(self):
        self.orchestrator: AgentOrchestrator = None
        self.config: Dict[str, Any] = {
            'ue_tcp_host': '127.0.0.1',
            'ue_tcp_port': 55557,
            'project_path': '',
            'widget_path': '/Game/Widgets',
            'cpp_header_paths': []
        }
        self.is_connected = False
        self.generation_history: List[Dict[str, Any]] = []
        self.active_sessions: Dict[str, Dict] = {}

app_state = AppState()

@app.route('/')
def index():
    """主页面"""
    return render_template('index.html')

@app.route('/api/config', methods=['GET', 'POST'])
def handle_config():
    """处理配置"""
    if request.method == 'GET':
        return jsonify(app_state.config)
    
    elif request.method == 'POST':
        try:
            new_config = request.json
            app_state.config.update(new_config)
            
            # 重新创建orchestrator
            create_orchestrator()
            
            return jsonify({'status': 'success', 'message': 'Configuration updated'})
        except Exception as e:
            return jsonify({'status': 'error', 'message': str(e)}), 400

@app.route('/api/test-connection', methods=['POST'])
def test_connection():
    """测试UE连接"""
    try:
        # 这里应该实现实际的连接测试
        import socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        
        host = app_state.config['ue_tcp_host']
        port = app_state.config['ue_tcp_port']
        result = sock.connect_ex((host, port))
        sock.close()
        
        app_state.is_connected = (result == 0)
        
        return jsonify({
            'status': 'success',
            'connected': app_state.is_connected,
            'message': 'Connected to Unreal Engine' if app_state.is_connected else 'Cannot connect to Unreal Engine'
        })
        
    except Exception as e:
        app_state.is_connected = False
        return jsonify({
            'status': 'error',
            'connected': False,
            'message': f'Connection test failed: {str(e)}'
        }), 500

@app.route('/api/history')
def get_history():
    """获取生成历史"""
    return jsonify(app_state.generation_history)

@app.route('/api/history', methods=['DELETE'])
def clear_history():
    """清空历史记录"""
    app_state.generation_history.clear()
    return jsonify({'status': 'success', 'message': 'History cleared'})

@socketio.on('connect')
def handle_connect():
    """客户端连接"""
    session_id = str(uuid.uuid4())
    session['session_id'] = session_id
    
    app_state.active_sessions[session_id] = {
        'connected_at': datetime.now(),
        'chat_history': []
    }
    
    emit('connected', {
        'session_id': session_id,
        'is_ue_connected': app_state.is_connected,
        'config': app_state.config
    })
    
    # 发送欢迎消息
    emit('chat_message', {
        'type': 'system',
        'content': 'Welcome to UMG Agent Web Interface! 🎮\nDescribe the UI you want to create, and I\'ll generate the corresponding Widget Blueprint in Unreal Engine.',
        'timestamp': datetime.now().isoformat()
    })

@socketio.on('disconnect')
def handle_disconnect():
    """客户端断开连接"""
    session_id = session.get('session_id')
    if session_id and session_id in app_state.active_sessions:
        del app_state.active_sessions[session_id]

@socketio.on('generate_ui')
def handle_generate_ui(data):
    """处理UI生成请求"""
    session_id = session.get('session_id')
    description = data.get('description', '').strip()
    
    if not description:
        emit('chat_message', {
            'type': 'error',
            'content': 'Please provide a UI description',
            'timestamp': datetime.now().isoformat()
        })
        return
    
    # 添加用户消息到聊天历史
    user_message = {
        'type': 'user',
        'content': description,
        'timestamp': datetime.now().isoformat()
    }
    
    if session_id in app_state.active_sessions:
        app_state.active_sessions[session_id]['chat_history'].append(user_message)
    
    emit('chat_message', user_message)
    
    # 显示生成中状态
    emit('chat_message', {
        'type': 'assistant',
        'content': '🔄 Generating UI...',
        'timestamp': datetime.now().isoformat(),
        'is_loading': True
    })
    
    # 异步生成UI
    def generate_ui_async():
        try:
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)
            
            result = loop.run_until_complete(async_generate_ui(description))
            
            # 发送结果
            socketio.emit('ui_generation_result', result, room=request.sid)
            
        except Exception as e:
            socketio.emit('ui_generation_result', {
                'status': 'error',
                'error': str(e)
            }, room=request.sid)
    
    # 在新线程中运行异步任务
    thread = threading.Thread(target=generate_ui_async)
    thread.daemon = True
    thread.start()

@socketio.on('test_connection')
def handle_test_connection_ws():
    """WebSocket连接测试"""
    try:
        import socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        
        host = app_state.config['ue_tcp_host']
        port = app_state.config['ue_tcp_port']
        result = sock.connect_ex((host, port))
        sock.close()
        
        app_state.is_connected = (result == 0)
        
        emit('connection_status', {
            'connected': app_state.is_connected,
            'message': 'Connected to Unreal Engine' if app_state.is_connected else 'Cannot connect to Unreal Engine'
        })
        
        # 添加系统消息
        emit('chat_message', {
            'type': 'system',
            'content': '✅ Connected to Unreal Engine' if app_state.is_connected else '❌ Cannot connect to Unreal Engine',
            'timestamp': datetime.now().isoformat()
        })
        
    except Exception as e:
        app_state.is_connected = False
        emit('connection_status', {
            'connected': False,
            'message': f'Connection test failed: {str(e)}'
        })

async def async_generate_ui(description: str) -> Dict[str, Any]:
    """异步生成UI"""
    try:
        if not app_state.orchestrator:
            create_orchestrator()
        
        # 调用实际的UI生成
        result = await app_state.orchestrator.execute_workflow(description)
        
        # 创建历史记录项
        history_item = {
            'id': str(uuid.uuid4()),
            'description': description,
            'status': result.status,
            'timestamp': datetime.now().isoformat(),
            'widget_path': result.execution_result.get('widget_blueprint_path', '') if result.execution_result else '',
            'components': result.execution_result.get('created_components', []) if result.execution_result else [],
            'component_count': len(result.execution_result.get('created_components', [])) if result.execution_result else 0,
            'recommendations': result.validation_report.get('recommendations', []) if result.validation_report else [],
            'errors': result.errors or []
        }
        
        app_state.generation_history.append(history_item)
        
        return {
            'status': 'success',
            'result': history_item
        }
        
    except Exception as e:
        error_item = {
            'id': str(uuid.uuid4()),
            'description': description,
            'status': 'error',
            'timestamp': datetime.now().isoformat(),
            'errors': [str(e)]
        }
        
        app_state.generation_history.append(error_item)
        
        return {
            'status': 'error',
            'error': str(e),
            'result': error_item
        }

def create_orchestrator():
    """创建编排器"""
    try:
        config = WorkflowConfig(
            ue_tcp_host=app_state.config['ue_tcp_host'],
            ue_tcp_port=app_state.config['ue_tcp_port']
        )
        app_state.orchestrator = AgentOrchestrator(config)
    except Exception as e:
        print(f"Failed to create orchestrator: {e}")
        app_state.orchestrator = None

def load_config_from_file():
    """从文件加载配置"""
    try:
        config_file = Path('config.json')
        if config_file.exists():
            with open(config_file, 'r', encoding='utf-8') as f:
                file_config = json.load(f)
            app_state.config.update(file_config)
            print(f"Configuration loaded from {config_file}")
    except Exception as e:
        print(f"Failed to load configuration: {e}")

def save_config_to_file():
    """保存配置到文件"""
    try:
        config_file = Path('config.json')
        with open(config_file, 'w', encoding='utf-8') as f:
            json.dump(app_state.config, f, indent=2, ensure_ascii=False)
        print(f"Configuration saved to {config_file}")
    except Exception as e:
        print(f"Failed to save configuration: {e}")

if __name__ == '__main__':
    print("🚀 Starting UMG Agent Web Interface...")
    
    # 加载配置
    load_config_from_file()
    
    # 创建orchestrator
    create_orchestrator()
    
    print(f"📡 Server will be available at: http://localhost:5000")
    print(f"🔗 UE Connection: {app_state.config['ue_tcp_host']}:{app_state.config['ue_tcp_port']}")
    
    # 启动服务器
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
