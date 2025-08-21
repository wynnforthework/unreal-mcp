"""
End-to-End Workflow Tests

Tests the complete agent chain workflow with various UI requirements scenarios.
"""

import pytest
import asyncio
import json
import tempfile
from pathlib import Path
from unittest.mock import Mock, AsyncMock, patch

from ..orchestrator import AgentOrchestrator, WorkflowConfig
from ..ui_parser_agent import UIParserAgent
from ..design_translator_agent import DesignTranslatorAgent
from ..ue_executor_agent import UEExecutorAgent
from ..binding_validator_agent import BindingValidatorAgent
from ..config import AgentConfig, ConfigManager


class TestEndToEndWorkflow:
    """Test the complete agent workflow end-to-end"""
    
    @pytest.fixture
    def config(self):
        """Create test configuration"""
        return WorkflowConfig(
            enable_rollback=True,
            max_retries_per_step=1,
            timeout_per_step=30.0,
            ue_tcp_host="127.0.0.1",
            ue_tcp_port=55557
        )
    
    @pytest.fixture
    def orchestrator(self, config):
        """Create orchestrator with test configuration"""
        return AgentOrchestrator(config)
    
    @pytest.fixture
    def mock_ue_responses(self):
        """Mock UE responses for testing"""
        return {
            "create_umg_widget_blueprint": {
                "success": True,
                "widget_path": "/Game/Widgets/WBP_TestWidget"
            },
            "add_widget_component": {
                "success": True,
                "component_name": "TestComponent"
            },
            "create_parent_and_child_widget_components": {
                "success": True,
                "parent_component": "ParentComponent",
                "child_component": "ChildComponent"
            },
            "bind_widget_event": {
                "success": True,
                "event_binding": "OnClicked"
            },
            "get_widget_component_layout": {
                "success": True,
                "hierarchy": {
                    "name": "RootCanvas",
                    "type": "CanvasPanel",
                    "children": [
                        {
                            "name": "Btn_StartGame",
                            "type": "Button",
                            "children": []
                        },
                        {
                            "name": "Txt_PlayerName",
                            "type": "TextBlock",
                            "children": []
                        }
                    ]
                }
            }
        }
    
    @pytest.mark.asyncio
    async def test_simple_menu_workflow(self, orchestrator, mock_ue_responses):
        """Test creating a simple game menu"""
        
        natural_language = "Create a main menu with a start game button and player name text"
        
        # Mock UE communication
        with patch.object(orchestrator.ue_executor, '_send_ue_command') as mock_ue:
            mock_ue.side_effect = lambda cmd, params: mock_ue_responses.get(cmd, {"success": False})
            
            result = await orchestrator.execute_workflow(natural_language)
        
        # Verify workflow completed successfully
        assert result.status == "success"
        assert result.ui_specification is not None
        assert result.umg_definition is not None
        assert result.execution_result is not None
        assert result.validation_report is not None
        
        # Verify UI specification
        ui_spec = result.ui_specification
        assert ui_spec["widget_name"].startswith("WBP_")
        assert len(ui_spec["components"]) >= 2  # At least button and text
        
        # Verify UMG definition
        umg_def = result.umg_definition
        assert umg_def["widget_name"] == ui_spec["widget_name"]
        assert "root_component" in umg_def
        
        # Verify execution result
        exec_result = result.execution_result
        assert exec_result["widget_blueprint_path"] is not None
        assert len(exec_result["created_components"]) > 0
    
    @pytest.mark.asyncio
    async def test_complex_ui_workflow(self, orchestrator, mock_ue_responses):
        """Test creating a complex UI with multiple components"""
        
        natural_language = """
        Create a player HUD with:
        - Health bar at the top left
        - Score display at the top right
        - Minimap in the bottom right corner
        - Chat box at the bottom left
        - Crosshair in the center
        """
        
        with patch.object(orchestrator.ue_executor, '_send_ue_command') as mock_ue:
            mock_ue.side_effect = lambda cmd, params: mock_ue_responses.get(cmd, {"success": False})
            
            result = await orchestrator.execute_workflow(natural_language)
        
        assert result.status == "success"
        
        # Verify multiple components were identified
        ui_spec = result.ui_specification
        assert len(ui_spec["components"]) >= 5  # Health, score, minimap, chat, crosshair
        
        # Verify component types
        component_types = [comp["type"] for comp in ui_spec["components"]]
        assert "ProgressBar" in component_types or "Slider" in component_types  # Health bar
        assert "TextBlock" in component_types  # Score display
        assert "Image" in component_types  # Minimap or crosshair
    
    @pytest.mark.asyncio
    async def test_workflow_with_cpp_validation(self, orchestrator, mock_ue_responses):
        """Test workflow with C++ binding validation"""
        
        # Create temporary C++ header file
        cpp_content = """
        #pragma once
        
        #include "CoreMinimal.h"
        #include "Blueprint/UserWidget.h"
        #include "Components/Button.h"
        #include "Components/TextBlock.h"
        #include "TestWidget.generated.h"
        
        UCLASS()
        class TESTPROJECT_API UTestWidget : public UUserWidget
        {
            GENERATED_BODY()
        
        public:
            UPROPERTY(meta = (BindWidget))
            class UButton* Btn_StartGame;
            
            UPROPERTY(meta = (BindWidget))
            class UTextBlock* Txt_PlayerName;
        };
        """
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
            f.write(cpp_content)
            cpp_path = f.name
        
        try:
            # Update orchestrator config
            orchestrator.config.cpp_header_path = cpp_path
            
            natural_language = "Create a widget with start game button and player name text"
            
            with patch.object(orchestrator.ue_executor, '_send_ue_command') as mock_ue:
                mock_ue.side_effect = lambda cmd, params: mock_ue_responses.get(cmd, {"success": False})
                
                result = await orchestrator.execute_workflow(natural_language)
            
            assert result.status == "success"
            
            # Verify validation report
            validation_report = result.validation_report
            assert validation_report is not None
            assert validation_report["overall_status"] in ["success", "warning"]
            
            # Check binding results
            binding_results = validation_report["binding_results"]
            assert len(binding_results) >= 2  # Button and text bindings
            
        finally:
            # Cleanup
            Path(cpp_path).unlink()
    
    @pytest.mark.asyncio
    async def test_workflow_error_handling(self, orchestrator):
        """Test workflow error handling and rollback"""
        
        natural_language = "Create a test widget"
        
        # Mock UE communication to fail at execution step
        with patch.object(orchestrator.ue_executor, '_send_ue_command') as mock_ue:
            mock_ue.return_value = {"success": False, "error": "UE connection failed"}
            
            result = await orchestrator.execute_workflow(natural_language)
        
        # Verify workflow failed gracefully
        assert result.status == "failed"
        assert len(result.errors) > 0
        assert "UE connection failed" in str(result.errors)
    
    @pytest.mark.asyncio
    async def test_workflow_timeout_handling(self, orchestrator):
        """Test workflow timeout handling"""
        
        # Set very short timeout
        orchestrator.config.timeout_per_step = 0.1
        
        natural_language = "Create a test widget"
        
        # Mock UE communication to be slow
        async def slow_ue_command(cmd, params):
            await asyncio.sleep(1.0)  # Longer than timeout
            return {"success": True}
        
        with patch.object(orchestrator.ue_executor, '_send_ue_command', side_effect=slow_ue_command):
            result = await orchestrator.execute_workflow(natural_language)
        
        # Verify workflow timed out
        assert result.status == "failed"
        assert any("timed out" in error.lower() for error in result.errors)
    
    @pytest.mark.asyncio
    async def test_workflow_progress_tracking(self, orchestrator, mock_ue_responses):
        """Test workflow progress tracking"""
        
        natural_language = "Create a simple widget"
        
        # Start workflow in background
        workflow_task = asyncio.create_task(
            orchestrator.execute_workflow(natural_language)
        )
        
        # Give it a moment to start
        await asyncio.sleep(0.1)
        
        # Check progress
        progress = orchestrator.get_workflow_progress()
        assert progress is not None
        assert "workflow_id" in progress
        assert "total_steps" in progress
        
        # Mock UE responses and let workflow complete
        with patch.object(orchestrator.ue_executor, '_send_ue_command') as mock_ue:
            mock_ue.side_effect = lambda cmd, params: mock_ue_responses.get(cmd, {"success": False})
            
            result = await workflow_task
        
        assert result.status == "success"


class TestIndividualAgents:
    """Test individual agents in isolation"""
    
    @pytest.mark.asyncio
    async def test_ui_parser_agent(self):
        """Test UI parser agent"""
        
        agent = UIParserAgent()
        
        input_data = {
            "natural_language_request": "Create a login form with username and password fields and a login button"
        }
        
        result = await agent.execute(input_data)
        
        assert result.is_success
        assert "ui_specification" in result.data
        
        ui_spec = result.data["ui_specification"]
        assert ui_spec["widget_name"].startswith("WBP_")
        assert len(ui_spec["components"]) >= 3  # Username, password, button
    
    @pytest.mark.asyncio
    async def test_design_translator_agent(self):
        """Test design translator agent"""
        
        agent = DesignTranslatorAgent()
        
        # Sample UI specification
        ui_spec = {
            "widget_name": "WBP_TestWidget",
            "components": [
                {
                    "id": "button1",
                    "type": "Button",
                    "label": "Start Game",
                    "position": {"x": 0.5, "y": 0.3},
                    "size": {"width": 200, "height": 50},
                    "properties": {},
                    "children": []
                },
                {
                    "id": "text1",
                    "type": "TextBlock",
                    "label": "Player Name",
                    "position": {"x": 0.1, "y": 0.1},
                    "size": {"width": 150, "height": 30},
                    "properties": {},
                    "children": []
                }
            ],
            "layout_type": "canvas"
        }
        
        input_data = {"ui_specification": ui_spec}
        
        result = await agent.execute(input_data)
        
        assert result.is_success
        assert "umg_widget_definition" in result.data
        
        umg_def = result.data["umg_widget_definition"]
        assert umg_def["widget_name"] == "WBP_TestWidget"
        assert "root_component" in umg_def
        assert "cpp_bindings" in umg_def
    
    @pytest.mark.asyncio
    async def test_binding_validator_agent(self):
        """Test binding validator agent"""
        
        agent = BindingValidatorAgent()
        
        # Create temporary C++ header
        cpp_content = """
        UPROPERTY(meta = (BindWidget))
        class UButton* Btn_TestButton;
        """
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
            f.write(cpp_content)
            cpp_path = f.name
        
        try:
            input_data = {
                "widget_blueprint_path": "/Game/Widgets/WBP_TestWidget",
                "cpp_header_path": cpp_path,
                "expected_bindings": [
                    {
                        "widget_name": "Btn_TestButton",
                        "cpp_variable_name": "Btn_TestButton",
                        "widget_type": "UButton"
                    }
                ]
            }
            
            # Mock UE communication
            with patch.object(agent, '_get_widget_layout') as mock_layout:
                mock_layout.return_value = {
                    "name": "RootCanvas",
                    "type": "CanvasPanel",
                    "children": [
                        {
                            "name": "Btn_TestButton",
                            "type": "Button",
                            "children": []
                        }
                    ]
                }
                
                result = await agent.execute(input_data)
            
            assert result.is_success
            assert "validation_report" in result.data
            
            validation_report = result.data["validation_report"]
            assert "overall_status" in validation_report
            assert "binding_results" in validation_report
            
        finally:
            Path(cpp_path).unlink()


class TestErrorScenarios:
    """Test various error scenarios"""
    
    @pytest.mark.asyncio
    async def test_invalid_natural_language(self, orchestrator):
        """Test handling of invalid natural language input"""
        
        # Empty input
        result = await orchestrator.execute_workflow("")
        assert result.status == "failed"
        
        # Very vague input
        result = await orchestrator.execute_workflow("make something")
        # Should still work but with low confidence
        assert result.ui_specification is not None
    
    @pytest.mark.asyncio
    async def test_ue_connection_failure(self, orchestrator):
        """Test handling of UE connection failures"""
        
        natural_language = "Create a test widget"
        
        # Mock connection failure
        with patch.object(orchestrator.ue_executor, '_send_ue_command') as mock_ue:
            mock_ue.side_effect = Exception("Connection refused")
            
            result = await orchestrator.execute_workflow(natural_language)
        
        assert result.status == "failed"
        assert any("Connection refused" in str(error) for error in result.errors)
    
    @pytest.mark.asyncio
    async def test_partial_workflow_failure(self, orchestrator, mock_ue_responses):
        """Test handling of partial workflow failures"""
        
        natural_language = "Create a test widget"
        
        # Mock UE to fail at binding step
        def mock_ue_command(cmd, params):
            if cmd == "bind_widget_event":
                return {"success": False, "error": "Binding failed"}
            return mock_ue_responses.get(cmd, {"success": False})
        
        with patch.object(orchestrator.ue_executor, '_send_ue_command', side_effect=mock_ue_command):
            result = await orchestrator.execute_workflow(natural_language)
        
        # Should still succeed overall but with warnings
        assert result.status in ["success", "partial"]
        assert result.execution_result is not None
