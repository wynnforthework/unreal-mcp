"""
Basic Usage Examples for UMG Agent System

This file demonstrates how to use the UMG agent system to generate
Widget Blueprints from natural language descriptions.
"""

import asyncio
import json
from pathlib import Path

from ..orchestrator import AgentOrchestrator, WorkflowConfig
from ..config import ConfigManager, AgentConfig
from ..tools.figma_ui_tool import FigmaUITool


async def example_basic_workflow():
    """Basic example of generating a UI from natural language"""
    
    print("=== Basic Workflow Example ===")
    
    # Create configuration
    config = WorkflowConfig(
        enable_rollback=True,
        max_retries_per_step=2,
        timeout_per_step=300.0,
        ue_tcp_host="127.0.0.1",
        ue_tcp_port=55557
    )
    
    # Create orchestrator
    orchestrator = AgentOrchestrator(config)
    
    # Natural language request
    natural_language = """
    Create a main menu for a game with:
    - A large "Start Game" button in the center
    - A "Settings" button below it
    - A "Quit" button at the bottom
    - The game title at the top
    """
    
    try:
        # Execute the workflow
        print("Executing workflow...")
        result = await orchestrator.execute_workflow(natural_language)
        
        # Print results
        print(f"Workflow Status: {result.status}")
        print(f"Execution Time: {result.execution_time:.2f} seconds")
        
        if result.status == "success":
            print(f"Widget Created: {result.execution_result['widget_blueprint_path']}")
            print(f"Components Created: {len(result.execution_result['created_components'])}")
            
            # Print validation results
            if result.validation_report:
                validation = result.validation_report
                print(f"Validation Status: {validation['overall_status']}")
                print(f"Binding Results: {len(validation['binding_results'])}")
        
        else:
            print("Errors:")
            for error in result.errors:
                print(f"  - {error}")
    
    except Exception as e:
        print(f"Error: {e}")


async def example_with_cpp_validation():
    """Example with C++ binding validation"""
    
    print("\n=== C++ Validation Example ===")
    
    # Create a sample C++ header file
    cpp_content = """
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MainMenuWidget.generated.h"

UCLASS()
class MYGAME_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UButton* Btn_StartGame;
    
    UPROPERTY(meta = (BindWidget))
    class UButton* Btn_Settings;
    
    UPROPERTY(meta = (BindWidget))
    class UButton* Btn_Quit;
    
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Txt_GameTitle;

protected:
    UFUNCTION()
    void OnStartGameClicked();
    
    UFUNCTION()
    void OnSettingsClicked();
    
    UFUNCTION()
    void OnQuitClicked();
};
"""
    
    # Write to temporary file
    cpp_path = Path("temp_main_menu.h")
    cpp_path.write_text(cpp_content)
    
    try:
        # Create configuration with C++ validation
        config = WorkflowConfig(
            enable_rollback=True,
            cpp_header_path=str(cpp_path),
            ue_tcp_host="127.0.0.1",
            ue_tcp_port=55557
        )
        
        orchestrator = AgentOrchestrator(config)
        
        natural_language = """
        Create a main menu widget with:
        - Start Game button
        - Settings button  
        - Quit button
        - Game Title text
        """
        
        print("Executing workflow with C++ validation...")
        result = await orchestrator.execute_workflow(natural_language)
        
        print(f"Workflow Status: {result.status}")
        
        if result.validation_report:
            validation = result.validation_report
            print(f"Validation Status: {validation['overall_status']}")
            
            print("Binding Results:")
            for binding in validation['binding_results']:
                status = binding['status']
                widget_name = binding['widget_name']
                print(f"  - {widget_name}: {status}")
            
            if validation['recommendations']:
                print("Recommendations:")
                for rec in validation['recommendations']:
                    print(f"  - {rec}")
    
    finally:
        # Cleanup
        if cpp_path.exists():
            cpp_path.unlink()


async def example_figma_integration():
    """Example of importing from Figma"""
    
    print("\n=== Figma Integration Example ===")
    
    # Note: This requires a valid Figma token and file URL
    figma_config = {
        "figma_token": "your_figma_token_here"  # Replace with actual token
    }
    
    figma_url = "https://www.figma.com/file/your_file_id/Design-Name"
    
    try:
        async with FigmaUITool(figma_config) as figma_tool:
            print("Fetching design from Figma...")
            
            # This would normally fetch from Figma
            # For demo purposes, we'll create a mock design
            mock_design = {
                "widget_name": "WBP_FigmaImport",
                "description": "Imported from Figma",
                "components": [
                    {
                        "id": "title",
                        "type": "TextBlock",
                        "label": "Welcome",
                        "position": {"x": 0.5, "y": 0.1},
                        "size": {"width": 200, "height": 40},
                        "properties": {"font_size": 24},
                        "children": []
                    },
                    {
                        "id": "login_btn",
                        "type": "Button", 
                        "label": "Login",
                        "position": {"x": 0.5, "y": 0.6},
                        "size": {"width": 150, "height": 50},
                        "properties": {},
                        "children": []
                    }
                ],
                "layout_type": "canvas",
                "interactions": [
                    {
                        "component_id": "login_btn",
                        "event_type": "OnClicked",
                        "action": "HandleLogin"
                    }
                ]
            }
            
            print("Design imported successfully!")
            print(f"Widget Name: {mock_design['widget_name']}")
            print(f"Components: {len(mock_design['components'])}")
            
            # Now create the widget using the orchestrator
            config = WorkflowConfig()
            orchestrator = AgentOrchestrator(config)
            
            # Convert design to natural language for processing
            natural_language = f"Create a widget based on imported design: {mock_design['description']}"
            
            print("Creating widget from Figma design...")
            result = await orchestrator.execute_workflow(
                natural_language,
                context={"source": "figma", "ui_specification": mock_design}
            )
            
            print(f"Widget creation status: {result.status}")
    
    except Exception as e:
        print(f"Figma integration error: {e}")
        print("Note: This example requires a valid Figma token and file URL")


async def example_batch_operations():
    """Example of batch operations using UE Python Tool"""
    
    print("\n=== Batch Operations Example ===")
    
    from ..tools.ue_python_tool import UEPythonTool
    
    # Create UE tool
    ue_tool = UEPythonTool()
    await ue_tool.start()
    
    try:
        # Define batch commands
        commands = [
            {
                "command_type": "create_umg_widget_blueprint",
                "params": {
                    "widget_name": "WBP_BatchTest1",
                    "parent_class": "UserWidget",
                    "path": "/Game/Widgets"
                }
            },
            {
                "command_type": "create_umg_widget_blueprint", 
                "params": {
                    "widget_name": "WBP_BatchTest2",
                    "parent_class": "UserWidget",
                    "path": "/Game/Widgets"
                }
            },
            {
                "command_type": "create_umg_widget_blueprint",
                "params": {
                    "widget_name": "WBP_BatchTest3",
                    "parent_class": "UserWidget",
                    "path": "/Game/Widgets"
                }
            }
        ]
        
        print("Executing batch commands...")
        results = await ue_tool.execute_batch(commands, parallel=True)
        
        print(f"Batch execution completed!")
        print(f"Total commands: {len(results)}")
        print(f"Successful: {sum(1 for r in results if r.success)}")
        print(f"Failed: {sum(1 for r in results if not r.success)}")
        
        # Print individual results
        for i, result in enumerate(results):
            status = "✓" if result.success else "✗"
            print(f"  Command {i+1}: {status} ({result.execution_time:.2f}s)")
    
    finally:
        await ue_tool.stop()


async def example_transaction_operations():
    """Example of transaction operations with rollback"""
    
    print("\n=== Transaction Operations Example ===")
    
    from ..tools.ue_python_tool import UEPythonTool
    
    ue_tool = UEPythonTool()
    await ue_tool.start()
    
    try:
        # Begin transaction
        print("Starting transaction...")
        tx_id = await ue_tool.begin_transaction()
        
        # Add commands to transaction with rollback commands
        ue_tool.add_to_transaction(
            "create_umg_widget_blueprint",
            {
                "widget_name": "WBP_TransactionTest",
                "parent_class": "UserWidget",
                "path": "/Game/Widgets"
            },
            rollback_command_type="delete_asset",
            rollback_params={"asset_path": "/Game/Widgets/WBP_TransactionTest"}
        )
        
        ue_tool.add_to_transaction(
            "add_widget_component",
            {
                "widget_name": "WBP_TransactionTest",
                "component_name": "Btn_Test",
                "component_type": "Button",
                "position": [100, 100],
                "size": [200, 50]
            }
        )
        
        # Commit transaction
        print("Committing transaction...")
        results = await ue_tool.commit_transaction()
        
        print(f"Transaction completed!")
        print(f"Commands executed: {len(results)}")
        
        all_success = all(r.success for r in results)
        print(f"All successful: {all_success}")
        
        if not all_success:
            print("Some commands failed - transaction was rolled back")
    
    except Exception as e:
        print(f"Transaction error: {e}")
    
    finally:
        await ue_tool.stop()


async def main():
    """Run all examples"""
    
    print("UMG Agent System Examples")
    print("=" * 50)
    
    # Run examples
    await example_basic_workflow()
    await example_with_cpp_validation()
    await example_figma_integration()
    await example_batch_operations()
    await example_transaction_operations()
    
    print("\n" + "=" * 50)
    print("Examples completed!")


if __name__ == "__main__":
    asyncio.run(main())
