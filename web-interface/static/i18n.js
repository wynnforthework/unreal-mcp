/**
 * 国际化配置文件
 * 支持中文（默认）和英文
 */

const i18nConfig = {
    // 默认语言
    defaultLanguage: 'zh',
    
    // 支持的语言
    languages: {
        'zh': '中文',
        'en': 'English'
    },
    
    // 翻译文本
    translations: {
        // 中文翻译
        'zh': {
            // 页面标题
            'page_title': 'Unreal MCP - 项目管理器',
            'header_title': 'Unreal MCP - 项目管理器',
            
            // 语言切换
            'language': '语言',
            
            // 系统状态
            'system_status': '系统状态',
            'python_status': 'Python',
            'agents_status': 'Agents',
            'ue_status': 'UE连接',
            
            // 主页面内容
            'welcome_title': '🚀 欢迎使用 Unreal MCP',
            'welcome_desc': '管理您的 Unreal Engine 项目并安装 MCP 工具，实现 AI 驱动的开发。选择现有项目或添加新项目以开始使用。',
            
            // 操作卡片
            'add_project_title': '添加新项目',
            'add_project_desc': '添加现有的 Unreal Engine 项目以开始使用 AI 助手的 MCP 工具。',
            'add_project_btn': '添加项目',
            
            'chat_interface_title': 'UMG 聊天界面',
            'chat_interface_desc': '使用聊天界面通过自然语言命令生成 UI 组件。',
            'open_chat_btn': '打开聊天',
            
            'documentation_title': '文档',
            'documentation_desc': '学习如何使用 Unreal MCP 工具并将它们与 AI 助手集成。',
            'view_docs_btn': '查看文档',
            
            // 项目管理
            'projects_section_title': '项目管理',
            'no_projects': '还没有项目。点击上方按钮添加您的第一个项目。',
            'refresh_projects': '刷新项目',
            
            // 项目状态
            'status_configured': '已配置',
            'status_installing': '安装中',
            'status_installed': '已安装',
            'status_error': '错误',
            
            // 项目操作按钮
            'install_mcp': '安装 MCP',
            'installing': '安装中...',
            'select_project': '选择',
            'start_servers': '启动服务器',
            'check_status': '检查状态',
            'open_folder': '打开文件夹',
            'remove_project': '移除',
            
            // 模态框
            'add_project_modal_title': '添加新项目',
            'project_path_label': '项目路径:',
            'project_path_placeholder': '输入 Unreal Engine 项目文件夹路径',
            'cancel': '取消',
            'add': '添加',
            
            // 通知消息
            'project_path_required': '请输入项目路径',
            'project_added': '项目添加成功',
            'project_selected': '项目已选择',
            'project_removed': '项目已移除',
            'installation_started': '安装已开始...',
            'installation_with_editor': '✅ 安装已开始！安装过程中将关闭 Unreal Editor。',
            'reinstall_with_editor': '✅ 重新安装已开始！安装过程中将关闭 Unreal Editor。',
            'servers_started': 'MCP 服务器启动成功！请检查您的终端窗口。',
            'starting_with_editor': '🔄 正在打开 Unreal Editor 并启动 MCP 服务器...',
            'python_required': '安装需要 Python 环境',
            'select_project_first': '请先选择一个项目',
            'remove_project_confirm': '您确定要从列表中移除此项目吗？',
            'documentation_coming_soon': '文档功能即将推出',
            'servers_running': 'MCP 服务器运行中',
            'no_servers_detected': '未检测到 MCP 服务器',
            'failed_start_servers': '启动 MCP 服务器失败',
            'failed_check_status': '检查服务器状态失败',
            'failed_add_project': '添加项目失败',
            'failed_remove_project': '移除项目失败',
            'unknown_error': '未知错误',
            
            // 项目信息
            'project_name': '项目名称',
            'project_path': '项目路径',
            'ue_version': 'UE 版本',
            'last_updated': '最后更新',
            'has_mcp_tools': 'MCP 工具',
            'yes': '是',
            'no': '否'
        },
        
        // 英文翻译
        'en': {
            // 页面标题
            'page_title': 'Unreal MCP - Project Manager',
            'header_title': 'Unreal MCP - Project Manager',
            
            // 语言切换
            'language': 'Language',
            
            // 系统状态
            'system_status': 'System Status',
            'python_status': 'Python',
            'agents_status': 'Agents',
            'ue_status': 'UE Connection',
            
            // 主页面内容
            'welcome_title': '🚀 Welcome to Unreal MCP',
            'welcome_desc': 'Manage your Unreal Engine projects and install MCP tools for AI-powered development. Select an existing project or add a new one to get started.',
            
            // 操作卡片
            'add_project_title': 'Add New Project',
            'add_project_desc': 'Add an existing Unreal Engine project to start using MCP tools with AI assistants.',
            'add_project_btn': 'Add Project',
            
            'chat_interface_title': 'UMG Chat Interface',
            'chat_interface_desc': 'Use the chat interface to generate UI components with natural language commands.',
            'open_chat_btn': 'Open Chat',
            
            'documentation_title': 'Documentation',
            'documentation_desc': 'Learn how to use Unreal MCP tools and integrate them with AI assistants.',
            'view_docs_btn': 'View Docs',
            
            // 项目管理
            'projects_section_title': 'Project Management',
            'no_projects': 'No projects yet. Click the button above to add your first project.',
            'refresh_projects': 'Refresh Projects',
            
            // 项目状态
            'status_configured': 'Configured',
            'status_installing': 'Installing',
            'status_installed': 'Installed',
            'status_error': 'Error',
            
            // 项目操作按钮
            'install_mcp': 'Install MCP',
            'installing': 'Installing...',
            'select_project': 'Select',
            'start_servers': 'Start Servers',
            'check_status': 'Check Status',
            'open_folder': 'Open Folder',
            'remove_project': 'Remove',
            
            // 模态框
            'add_project_modal_title': 'Add New Project',
            'project_path_label': 'Project Path:',
            'project_path_placeholder': 'Enter Unreal Engine project folder path',
            'cancel': 'Cancel',
            'add': 'Add',
            
            // 通知消息
            'project_path_required': 'Please enter a project path',
            'project_added': 'Project added successfully',
            'project_selected': 'Project selected',
            'project_removed': 'Project removed',
            'installation_started': 'Installation started...',
            'installation_with_editor': '✅ Installation started! Unreal Editor will be closed during installation.',
            'reinstall_with_editor': '✅ Reinstallation started! Unreal Editor will be closed during installation.',
            'servers_started': 'MCP servers started successfully! Check your terminal windows.',
            'starting_with_editor': '🔄 Opening Unreal Editor and starting MCP servers...',
            'python_required': 'Python is required for installation',
            'select_project_first': 'Please select a project first',
            'remove_project_confirm': 'Are you sure you want to remove this project from the list?',
            'documentation_coming_soon': 'Documentation feature coming soon',
            'servers_running': 'MCP servers running',
            'no_servers_detected': 'No MCP servers detected',
            'failed_start_servers': 'Failed to start MCP servers',
            'failed_check_status': 'Failed to check server status',
            'failed_add_project': 'Failed to add project',
            'failed_remove_project': 'Failed to remove project',
            'unknown_error': 'Unknown error',
            
            // 项目信息
            'project_name': 'Project Name',
            'project_path': 'Project Path',
            'ue_version': 'UE Version',
            'last_updated': 'Last Updated',
            'has_mcp_tools': 'MCP Tools',
            'yes': 'Yes',
            'no': 'No'
        }
    }
};

// 国际化管理器
class I18nManager {
    constructor() {
        this.currentLanguage = this.loadLanguage();
        this.translations = i18nConfig.translations;
        this.supportedLanguages = i18nConfig.languages;
    }
    
    // 从本地存储加载语言设置
    loadLanguage() {
        const savedLang = localStorage.getItem('preferred_language');
        return savedLang || i18nConfig.defaultLanguage;
    }
    
    // 保存语言设置到本地存储
    saveLanguage(lang) {
        localStorage.setItem('preferred_language', lang);
        this.currentLanguage = lang;
    }
    
    // 获取翻译文本
    t(key, defaultText = '') {
        const translation = this.translations[this.currentLanguage]?.[key];
        return translation || defaultText || key;
    }
    
    // 切换语言
    switchLanguage(lang) {
        if (this.supportedLanguages[lang]) {
            this.saveLanguage(lang);
            this.updatePageTexts();
            this.updatePageLanguage();
        }
    }
    
    // 更新页面语言属性
    updatePageLanguage() {
        document.documentElement.lang = this.currentLanguage === 'zh' ? 'zh-CN' : 'en';
        document.title = this.t('page_title');
    }
    
    // 更新页面中所有可翻译的文本
    updatePageTexts() {
        // 更新所有带有 data-i18n 属性的元素
        document.querySelectorAll('[data-i18n]').forEach(element => {
            const key = element.getAttribute('data-i18n');
            element.textContent = this.t(key);
        });
        
        // 更新所有带有 data-i18n-placeholder 属性的输入框
        document.querySelectorAll('[data-i18n-placeholder]').forEach(element => {
            const key = element.getAttribute('data-i18n-placeholder');
            element.placeholder = this.t(key);
        });
        
        // 更新所有带有 data-i18n-title 属性的元素
        document.querySelectorAll('[data-i18n-title]').forEach(element => {
            const key = element.getAttribute('data-i18n-title');
            element.title = this.t(key);
        });
    }
    
    // 创建语言切换器
    createLanguageSwitcher() {
        const switcher = document.createElement('div');
        switcher.className = 'language-switcher';
        switcher.innerHTML = `
            <label data-i18n="language">语言</label>:
            <select id="languageSelect">
                ${Object.entries(this.supportedLanguages).map(([code, name]) => 
                    `<option value="${code}" ${code === this.currentLanguage ? 'selected' : ''}>${name}</option>`
                ).join('')}
            </select>
        `;
        
        // 添加事件监听器
        const select = switcher.querySelector('#languageSelect');
        select.addEventListener('change', (e) => {
            this.switchLanguage(e.target.value);
        });
        
        return switcher;
    }
    
    // 初始化国际化
    init() {
        this.updatePageLanguage();
        this.updatePageTexts();
        
        // 添加语言切换器到页面
        const headerControls = document.querySelector('.header-controls');
        if (headerControls) {
            headerControls.appendChild(this.createLanguageSwitcher());
        }
    }
}

// 全局实例
const i18n = new I18nManager();
