"""
UMG Agent System for Unreal MCP

This package contains the 4-agent system for automated UMG Widget Blueprint generation:
1. UI Requirements Parser Agent - Natural language to UI JSON schema
2. Design Translation Agent - UI schema to UMG-compatible JSON
3. UE Editor Execution Agent - UMG JSON to actual Widget Blueprints
4. Binding Validation Agent - C++ binding validation and reporting

The agents work together in a coordinated workflow to transform natural language
UI requirements into fully functional UMG Widget Blueprints in Unreal Engine.
"""

from .base_agent import BaseAgent, AgentResult, AgentError
from .ui_parser_agent import UIParserAgent
from .design_translator_agent import DesignTranslatorAgent
from .ue_executor_agent import UEExecutorAgent
from .binding_validator_agent import BindingValidatorAgent
from .orchestrator import AgentOrchestrator

__all__ = [
    'BaseAgent',
    'AgentResult', 
    'AgentError',
    'UIParserAgent',
    'DesignTranslatorAgent', 
    'UEExecutorAgent',
    'BindingValidatorAgent',
    'AgentOrchestrator'
]
