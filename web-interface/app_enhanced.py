#!/usr/bin/env python3
"""
增强版 UMG Agent Web Interface
集成项目管理和一键安装功能
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

# 导入安装管理器
from install_manager import InstallManager, ProjectInfo

# 添加agents目录到Python路径
agents_path = Path(__file__).parent.parent / "Python" / "agents"
sys.path.insert(0, str(agents_path))

print(f"📁 Agents path: {agents_path}")

# 尝试导入agents系统
try:
    if agents_path.exists():
        from orchestrator import AgentOrchestrator, WorkflowConfig
        from config import ConfigManager, AgentConfig
        print("✅ UMG Agent modules imported successfully")
        AGENTS_AVAILABLE = True
    else:
        print("⚠️  Agents directory not found - UMG features will be limited")
        AGENTS_AVAILABLE = False
except ImportError as e:
    print(f"⚠️  UMG Agent modules not available: {e}")
    AGENTS_AVAILABLE = False

app = Flask(__name__)
app.config['SECRET_KEY'] = 'umg-agent-secret-key'
socketio = SocketIO(app, cors_allowed_origins="*")

# 全局状态
class AppState:
    def __init__(self):
        self.orchestrator: Any = None
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
        self.current_project: str = ""

app_state = AppState()
install_manager = InstallManager()

@app.route('/')
def index():
    """主页面 - 项目管理"""
    return render_template('index_enhanced.html')

@app.route('/chat')
def chat():
    """聊天界面"""
    return render_template('chat.html')

# ============ 项目管理 API ============

@app.route('/api/projects', methods=['GET'])
def get_projects():
    """获取项目列表"""
    projects = install_manager.get_projects_list()
    return jsonify(projects)

@app.route('/api/projects', methods=['POST'])
def add_project():
    """添加项目"""
    data = request.json
    project_path = data.get('path', '').strip()
    
    if not project_path:
        return jsonify({'status': 'error', 'message': 'Project path is required'}), 400
    
    success, message = install_manager.add_project(project_path)
    
    if success:
        return jsonify({'status': 'success', 'message': message})
    else:
        return jsonify({'status': 'error', 'message': message}), 400

@app.route('/api/projects/<path:project_path>', methods=['DELETE'])
def remove_project(project_path):
    """移除项目"""
    success, message = install_manager.remove_project(project_path)
    
    if success:
        return jsonify({'status': 'success', 'message': message})
    else:
        return jsonify({'status': 'error', 'message': message}), 404

@app.route('/api/projects/<path:project_path>/install', methods=['POST'])
def install_project(project_path):
    """安装 MCP 工具到项目"""
    # 异步安装
    def install_async():
        try:
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)
            
            success, message = loop.run_until_complete(
                install_manager.install_mcp_tools(project_path)
            )
            
            # 通过 WebSocket 发送安装结果
            socketio.emit('installation_result', {
                'project_path': project_path,
                'success': success,
                'message': message
            })
            
        except Exception as e:
            socketio.emit('installation_result', {
                'project_path': project_path,
                'success': False,
                'message': str(e)
            })
    
    # 在新线程中运行安装
    thread = threading.Thread(target=install_async)
    thread.daemon = True
    thread.start()
    
    return jsonify({'status': 'success', 'message': 'Installation started'})

@app.route('/api/projects/<path:project_path>/status')
def get_project_status(project_path):
    """获取项目安装状态"""
    status = install_manager.get_installation_status(project_path)
    return jsonify(status)

@app.route('/api/projects/<path:project_path>/select', methods=['POST'])
def select_project(project_path):
    """选择当前工作项目"""
    if project_path in install_manager.projects:
        app_state.current_project = project_path
        app_state.config['project_path'] = project_path
        
        # 如果项目已安装 MCP 工具，重新创建 orchestrator
        if AGENTS_AVAILABLE and install_manager.projects[project_path].has_mcp_tools:
            create_orchestrator()
        
        return jsonify({'status': 'success', 'message': 'Project selected'})
    else:
        return jsonify({'status': 'error', 'message': 'Project not found'}), 404

@app.route('/api/projects/<path:project_path>/start-servers', methods=['POST'])
def start_mcp_servers(project_path):
    """启动项目的 MCP 服务器"""
    try:
        print(f"🚀 Starting MCP servers for project: {project_path}")
        
        # 检查项目路径是否存在
        if project_path not in install_manager.projects:
            return jsonify({'status': 'error', 'message': 'Project not found'}), 404
        
        success, message = install_manager.start_mcp_servers(project_path)
        
        if success:
            print(f"✅ MCP servers started successfully for: {project_path}")
            return jsonify({'status': 'success', 'message': message})
        else:
            print(f"❌ Failed to start MCP servers for: {project_path} - {message}")
            return jsonify({'status': 'error', 'message': message}), 400
            
    except Exception as e:
        print(f"💥 Exception in start_mcp_servers: {str(e)}")
        return jsonify({'status': 'error', 'message': f'Server error: {str(e)}'}), 500

@app.route('/api/projects/<path:project_path>/server-status')
def get_server_status(project_path):
    """获取 MCP 服务器运行状态"""
    running, servers = install_manager.check_mcp_servers_running(project_path)
    
    return jsonify({
        'running': running,
        'servers': servers,
        'total_servers': len(servers)
    })

@app.route('/api/system/check')
def check_system():
    """检查系统环境"""
    python_ok, python_msg = install_manager.check_python_dependencies()
    
    return jsonify({
        'python': {
            'available': python_ok,
            'message': python_msg
        },
        'agents': {
            'available': AGENTS_AVAILABLE,
            'message': 'UMG Agents available' if AGENTS_AVAILABLE else 'UMG Agents not available'
        }
    })

# ============ 原有 UMG 功能 API ============

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
            if AGENTS_AVAILABLE:
                create_orchestrator()
            
            return jsonify({'status': 'success', 'message': 'Configuration updated'})
        except Exception as e:
            return jsonify({'status': 'error', 'message': str(e)}), 400

@app.route('/api/test-connection', methods=['POST'])
def test_connection():
    """测试UE连接"""
    try:
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

# ============ WebSocket 事件 ============

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
        'config': app_state.config,
        'current_project': app_state.current_project,
        'agents_available': AGENTS_AVAILABLE
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
    if not AGENTS_AVAILABLE:
        emit('chat_message', {
            'type': 'error',
            'content': 'UMG Agent system is not available. Please ensure the agents are properly installed.',
            'timestamp': datetime.now().isoformat()
        })
        return
    
    session_id = session.get('session_id')
    description = data.get('description', '').strip()
    
    if not description:
        emit('chat_message', {
            'type': 'error',
            'content': 'Please provide a UI description',
            'timestamp': datetime.now().isoformat()
        })
        return
    
    # 检查是否选择了项目
    if not app_state.current_project:
        emit('chat_message', {
            'type': 'error',
            'content': 'Please select a project first',
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
        
    except Exception as e:
        app_state.is_connected = False
        emit('connection_status', {
            'connected': False,
            'message': f'Connection test failed: {str(e)}'
        })

# ============ 辅助函数 ============

async def async_generate_ui(description: str) -> Dict[str, Any]:
    """异步生成UI"""
    if not AGENTS_AVAILABLE:
        return {
            'status': 'error',
            'error': 'UMG Agent system is not available'
        }
    
    try:
        if not app_state.orchestrator:
            create_orchestrator()
        
        if not app_state.orchestrator:
            return {
                'status': 'error',
                'error': 'Failed to create orchestrator'
            }
        
        # 调用实际的UI生成
        result = await app_state.orchestrator.execute_workflow(description)
        
        # 创建历史记录项
        history_item = {
            'id': str(uuid.uuid4()),
            'description': description,
            'status': result.status,
            'timestamp': datetime.now().isoformat(),
            'project_path': app_state.current_project,
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
            'project_path': app_state.current_project,
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
    if not AGENTS_AVAILABLE:
        return
    
    try:
        config = WorkflowConfig(
            ue_tcp_host=app_state.config['ue_tcp_host'],
            ue_tcp_port=app_state.config['ue_tcp_port']
        )
        app_state.orchestrator = AgentOrchestrator(config)
        print("✅ Orchestrator created successfully")
    except Exception as e:
        print(f"❌ Failed to create orchestrator: {e}")
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

if __name__ == '__main__':
    print("🚀 Starting Enhanced UMG Agent Web Interface...")
    
    # 加载配置
    load_config_from_file()
    
    # 创建orchestrator
    if AGENTS_AVAILABLE:
        create_orchestrator()
    
    print(f"📡 Server will be available at: http://localhost:5000")
    print(f"🔗 UE Connection: {app_state.config['ue_tcp_host']}:{app_state.config['ue_tcp_port']}")
    print(f"🤖 UMG Agents: {'Available' if AGENTS_AVAILABLE else 'Not Available'}")
    
    # 启动服务器
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
