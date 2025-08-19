"""
Base Agent Architecture for UMG Agent System

Defines the foundational interfaces and data structures for all agents
in the UMG Widget Blueprint generation workflow.
"""

from abc import ABC, abstractmethod
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional, Union
from enum import Enum
import json
import logging
from datetime import datetime


class AgentStatus(Enum):
    """Status of agent execution"""
    PENDING = "pending"
    RUNNING = "running" 
    SUCCESS = "success"
    ERROR = "error"
    CANCELLED = "cancelled"


@dataclass
class AgentError:
    """Represents an error that occurred during agent execution"""
    code: str
    message: str
    details: Optional[Dict[str, Any]] = None
    timestamp: datetime = field(default_factory=datetime.now)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "code": self.code,
            "message": self.message,
            "details": self.details,
            "timestamp": self.timestamp.isoformat()
        }


@dataclass
class AgentResult:
    """Result of agent execution with status, data, and metadata"""
    status: AgentStatus
    data: Optional[Dict[str, Any]] = None
    error: Optional[AgentError] = None
    metadata: Dict[str, Any] = field(default_factory=dict)
    execution_time: Optional[float] = None
    timestamp: datetime = field(default_factory=datetime.now)
    
    @property
    def is_success(self) -> bool:
        return self.status == AgentStatus.SUCCESS
    
    @property
    def is_error(self) -> bool:
        return self.status == AgentStatus.ERROR
    
    def to_dict(self) -> Dict[str, Any]:
        result = {
            "status": self.status.value,
            "data": self.data,
            "metadata": self.metadata,
            "execution_time": self.execution_time,
            "timestamp": self.timestamp.isoformat()
        }
        if self.error:
            result["error"] = self.error.to_dict()
        return result


class BaseAgent(ABC):
    """
    Base class for all agents in the UMG generation workflow.
    
    Each agent implements a specific step in the UI generation pipeline:
    - UI Parser: Natural language → UI JSON schema
    - Design Translator: UI schema → UMG JSON
    - UE Executor: UMG JSON → Widget Blueprint
    - Binding Validator: Blueprint + C++ → Validation report
    """
    
    def __init__(self, name: str, config: Optional[Dict[str, Any]] = None):
        self.name = name
        self.config = config or {}
        self.logger = logging.getLogger(f"agent.{name}")
        self._setup_logging()
    
    def _setup_logging(self):
        """Setup logging for the agent"""
        if not self.logger.handlers:
            handler = logging.StreamHandler()
            formatter = logging.Formatter(
                f'%(asctime)s - {self.name} - %(levelname)s - %(message)s'
            )
            handler.setFormatter(formatter)
            self.logger.addHandler(handler)
            self.logger.setLevel(logging.INFO)
    
    @abstractmethod
    async def execute(self, input_data: Dict[str, Any]) -> AgentResult:
        """
        Execute the agent's main functionality.
        
        Args:
            input_data: Input data for the agent
            
        Returns:
            AgentResult with status, output data, and metadata
        """
        pass
    
    @abstractmethod
    def validate_input(self, input_data: Dict[str, Any]) -> bool:
        """
        Validate input data format and requirements.
        
        Args:
            input_data: Input data to validate
            
        Returns:
            True if valid, False otherwise
        """
        pass
    
    def get_schema(self) -> Dict[str, Any]:
        """
        Get the JSON schema for this agent's input/output format.
        
        Returns:
            JSON schema dictionary
        """
        return {
            "input_schema": self._get_input_schema(),
            "output_schema": self._get_output_schema()
        }
    
    @abstractmethod
    def _get_input_schema(self) -> Dict[str, Any]:
        """Get the input JSON schema for this agent"""
        pass
    
    @abstractmethod
    def _get_output_schema(self) -> Dict[str, Any]:
        """Get the output JSON schema for this agent"""
        pass
    
    def create_success_result(self, data: Dict[str, Any], 
                            metadata: Optional[Dict[str, Any]] = None,
                            execution_time: Optional[float] = None) -> AgentResult:
        """Create a successful result"""
        return AgentResult(
            status=AgentStatus.SUCCESS,
            data=data,
            metadata=metadata or {},
            execution_time=execution_time
        )
    
    def create_error_result(self, error_code: str, error_message: str,
                          error_details: Optional[Dict[str, Any]] = None,
                          execution_time: Optional[float] = None) -> AgentResult:
        """Create an error result"""
        error = AgentError(
            code=error_code,
            message=error_message,
            details=error_details
        )
        return AgentResult(
            status=AgentStatus.ERROR,
            error=error,
            execution_time=execution_time
        )


# Common data schemas used across agents
UI_COMPONENT_SCHEMA = {
    "type": "object",
    "properties": {
        "name": {"type": "string"},
        "type": {"type": "string"},
        "position": {
            "type": "object",
            "properties": {
                "x": {"type": "number"},
                "y": {"type": "number"}
            }
        },
        "size": {
            "type": "object", 
            "properties": {
                "width": {"type": "number"},
                "height": {"type": "number"}
            }
        },
        "properties": {"type": "object"},
        "children": {
            "type": "array",
            "items": {"$ref": "#"}
        }
    },
    "required": ["name", "type"]
}

UI_LAYOUT_SCHEMA = {
    "type": "object",
    "properties": {
        "widget_name": {"type": "string"},
        "description": {"type": "string"},
        "root_component": UI_COMPONENT_SCHEMA,
        "metadata": {"type": "object"}
    },
    "required": ["widget_name", "root_component"]
}
