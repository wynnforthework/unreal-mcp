"""
Agent Configuration and Management

Implements configuration management for agents, including settings for LLM models,
validation rules, UE project paths, and agent behavior customization.
Includes monitoring and logging capabilities.
"""

import json
import os
import logging
from typing import Dict, Any, Optional, List
from dataclasses import dataclass, field, asdict
from pathlib import Path
from datetime import datetime
import yaml


@dataclass
class LLMConfig:
    """Configuration for LLM integration"""
    provider: str = "openai"  # openai, anthropic, local, etc.
    model: str = "gpt-4"
    api_key: Optional[str] = None
    base_url: Optional[str] = None
    temperature: float = 0.7
    max_tokens: int = 2048
    timeout: float = 30.0
    retry_attempts: int = 3


@dataclass
class UEConfig:
    """Configuration for Unreal Engine integration"""
    tcp_host: str = "127.0.0.1"
    tcp_port: int = 55557
    connection_timeout: float = 10.0
    command_timeout: float = 30.0
    max_retries: int = 3
    project_path: Optional[str] = None
    widget_path: str = "/Game/Widgets"
    cpp_header_paths: List[str] = field(default_factory=list)


@dataclass
class FigmaConfig:
    """Configuration for Figma integration"""
    api_token: Optional[str] = None
    base_url: str = "https://api.figma.com/v1"
    timeout: float = 30.0
    cache_enabled: bool = True
    cache_duration: int = 3600  # seconds


@dataclass
class ValidationConfig:
    """Configuration for validation rules"""
    strict_naming: bool = True
    require_cpp_bindings: bool = False
    allow_missing_widgets: bool = True
    max_component_depth: int = 10
    validate_widget_types: bool = True
    custom_validation_rules: Dict[str, Any] = field(default_factory=dict)


@dataclass
class LoggingConfig:
    """Configuration for logging"""
    level: str = "INFO"
    format: str = "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
    file_path: Optional[str] = None
    max_file_size: int = 10 * 1024 * 1024  # 10MB
    backup_count: int = 5
    console_output: bool = True


@dataclass
class MonitoringConfig:
    """Configuration for monitoring and metrics"""
    enabled: bool = True
    metrics_interval: float = 60.0  # seconds
    performance_tracking: bool = True
    error_tracking: bool = True
    export_metrics: bool = False
    metrics_file: Optional[str] = None


@dataclass
class AgentConfig:
    """Main configuration for the agent system"""
    llm: LLMConfig = field(default_factory=LLMConfig)
    ue: UEConfig = field(default_factory=UEConfig)
    figma: FigmaConfig = field(default_factory=FigmaConfig)
    validation: ValidationConfig = field(default_factory=ValidationConfig)
    logging: LoggingConfig = field(default_factory=LoggingConfig)
    monitoring: MonitoringConfig = field(default_factory=MonitoringConfig)
    
    # Workflow settings
    enable_rollback: bool = True
    max_retries_per_step: int = 2
    timeout_per_step: float = 300.0
    parallel_validation: bool = False
    auto_fix_errors: bool = False
    
    # Custom settings
    custom_settings: Dict[str, Any] = field(default_factory=dict)


class ConfigManager:
    """
    Manages configuration for the agent system.
    
    Supports loading from files, environment variables, and runtime updates.
    Provides validation and monitoring capabilities.
    """
    
    def __init__(self, config_path: Optional[str] = None):
        self.config_path = config_path
        self.config = AgentConfig()
        self.logger = logging.getLogger("config_manager")
        
        # Load configuration
        if config_path:
            self.load_from_file(config_path)
        
        self.load_from_environment()
        self.setup_logging()
    
    def load_from_file(self, file_path: str):
        """Load configuration from a file (JSON or YAML)"""
        
        path = Path(file_path)
        if not path.exists():
            self.logger.warning(f"Configuration file not found: {file_path}")
            return
        
        try:
            with open(path, 'r', encoding='utf-8') as f:
                if path.suffix.lower() in ['.yaml', '.yml']:
                    data = yaml.safe_load(f)
                else:
                    data = json.load(f)
            
            self._update_config_from_dict(data)
            self.logger.info(f"Configuration loaded from {file_path}")
            
        except Exception as e:
            self.logger.error(f"Failed to load configuration from {file_path}: {e}")
    
    def load_from_environment(self):
        """Load configuration from environment variables"""
        
        env_mappings = {
            # LLM settings
            "AGENT_LLM_PROVIDER": ("llm", "provider"),
            "AGENT_LLM_MODEL": ("llm", "model"),
            "AGENT_LLM_API_KEY": ("llm", "api_key"),
            "AGENT_LLM_BASE_URL": ("llm", "base_url"),
            "AGENT_LLM_TEMPERATURE": ("llm", "temperature", float),
            
            # UE settings
            "AGENT_UE_HOST": ("ue", "tcp_host"),
            "AGENT_UE_PORT": ("ue", "tcp_port", int),
            "AGENT_UE_PROJECT_PATH": ("ue", "project_path"),
            "AGENT_UE_WIDGET_PATH": ("ue", "widget_path"),
            
            # Figma settings
            "AGENT_FIGMA_TOKEN": ("figma", "api_token"),
            
            # Logging settings
            "AGENT_LOG_LEVEL": ("logging", "level"),
            "AGENT_LOG_FILE": ("logging", "file_path"),
        }
        
        for env_var, config_path in env_mappings.items():
            value = os.getenv(env_var)
            if value is not None:
                # Convert type if specified
                if len(config_path) > 2:
                    converter = config_path[2]
                    try:
                        value = converter(value)
                    except ValueError:
                        self.logger.warning(f"Invalid value for {env_var}: {value}")
                        continue
                
                # Set configuration value
                self._set_nested_config(config_path[0], config_path[1], value)
    
    def save_to_file(self, file_path: str, format: str = "auto"):
        """Save configuration to a file"""
        
        path = Path(file_path)
        
        # Determine format
        if format == "auto":
            format = "yaml" if path.suffix.lower() in ['.yaml', '.yml'] else "json"
        
        try:
            # Create directory if it doesn't exist
            path.parent.mkdir(parents=True, exist_ok=True)
            
            # Convert config to dict
            config_dict = asdict(self.config)
            
            with open(path, 'w', encoding='utf-8') as f:
                if format == "yaml":
                    yaml.dump(config_dict, f, default_flow_style=False, indent=2)
                else:
                    json.dump(config_dict, f, indent=2, default=str)
            
            self.logger.info(f"Configuration saved to {file_path}")
            
        except Exception as e:
            self.logger.error(f"Failed to save configuration to {file_path}: {e}")
    
    def update_config(self, updates: Dict[str, Any]):
        """Update configuration with new values"""
        
        self._update_config_from_dict(updates)
        self.logger.info("Configuration updated")
    
    def get_config(self) -> AgentConfig:
        """Get the current configuration"""
        return self.config
    
    def validate_config(self) -> List[str]:
        """Validate the current configuration and return any issues"""
        
        issues = []
        
        # Validate LLM config
        if not self.config.llm.provider:
            issues.append("LLM provider not specified")
        
        if not self.config.llm.model:
            issues.append("LLM model not specified")
        
        # Validate UE config
        if self.config.ue.tcp_port <= 0 or self.config.ue.tcp_port > 65535:
            issues.append("Invalid UE TCP port")
        
        # Validate paths
        if self.config.ue.project_path and not Path(self.config.ue.project_path).exists():
            issues.append(f"UE project path does not exist: {self.config.ue.project_path}")
        
        for cpp_path in self.config.ue.cpp_header_paths:
            if not Path(cpp_path).exists():
                issues.append(f"C++ header path does not exist: {cpp_path}")
        
        # Validate logging config
        if self.config.logging.level not in ["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"]:
            issues.append("Invalid logging level")
        
        return issues
    
    def setup_logging(self):
        """Setup logging based on configuration"""
        
        # Configure root logger
        logger = logging.getLogger()
        logger.setLevel(getattr(logging, self.config.logging.level))
        
        # Clear existing handlers
        logger.handlers.clear()
        
        # Create formatter
        formatter = logging.Formatter(self.config.logging.format)
        
        # Console handler
        if self.config.logging.console_output:
            console_handler = logging.StreamHandler()
            console_handler.setFormatter(formatter)
            logger.addHandler(console_handler)
        
        # File handler
        if self.config.logging.file_path:
            try:
                from logging.handlers import RotatingFileHandler
                
                # Create directory if it doesn't exist
                log_path = Path(self.config.logging.file_path)
                log_path.parent.mkdir(parents=True, exist_ok=True)
                
                file_handler = RotatingFileHandler(
                    self.config.logging.file_path,
                    maxBytes=self.config.logging.max_file_size,
                    backupCount=self.config.logging.backup_count
                )
                file_handler.setFormatter(formatter)
                logger.addHandler(file_handler)
                
            except Exception as e:
                self.logger.error(f"Failed to setup file logging: {e}")
    
    def _update_config_from_dict(self, data: Dict[str, Any]):
        """Update configuration from dictionary"""
        
        for key, value in data.items():
            if hasattr(self.config, key):
                if isinstance(value, dict):
                    # Update nested configuration
                    config_obj = getattr(self.config, key)
                    for sub_key, sub_value in value.items():
                        if hasattr(config_obj, sub_key):
                            setattr(config_obj, sub_key, sub_value)
                else:
                    setattr(self.config, key, value)
    
    def _set_nested_config(self, section: str, key: str, value: Any):
        """Set a nested configuration value"""
        
        if hasattr(self.config, section):
            config_obj = getattr(self.config, section)
            if hasattr(config_obj, key):
                setattr(config_obj, key, value)
    
    def get_monitoring_config(self) -> MonitoringConfig:
        """Get monitoring configuration"""
        return self.config.monitoring
    
    def export_config_template(self, file_path: str):
        """Export a configuration template with default values"""
        
        template_config = AgentConfig()
        
        # Add comments to the template
        config_dict = asdict(template_config)
        
        # Add documentation
        config_dict["_documentation"] = {
            "llm": "Configuration for LLM integration (OpenAI, Anthropic, etc.)",
            "ue": "Configuration for Unreal Engine integration",
            "figma": "Configuration for Figma API integration",
            "validation": "Validation rules and settings",
            "logging": "Logging configuration",
            "monitoring": "Monitoring and metrics configuration"
        }
        
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                if file_path.endswith('.yaml') or file_path.endswith('.yml'):
                    yaml.dump(config_dict, f, default_flow_style=False, indent=2)
                else:
                    json.dump(config_dict, f, indent=2, default=str)
            
            self.logger.info(f"Configuration template exported to {file_path}")
            
        except Exception as e:
            self.logger.error(f"Failed to export configuration template: {e}")


# Global configuration manager instance
_config_manager: Optional[ConfigManager] = None


def get_config_manager(config_path: Optional[str] = None) -> ConfigManager:
    """Get the global configuration manager instance"""
    global _config_manager
    
    if _config_manager is None:
        _config_manager = ConfigManager(config_path)
    
    return _config_manager


def get_config() -> AgentConfig:
    """Get the current configuration"""
    return get_config_manager().get_config()
