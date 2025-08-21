# UMG Agent System

A comprehensive 4-agent system for automated UMG Widget Blueprint generation in Unreal Engine 5.6. This system transforms natural language UI requirements into fully functional Widget Blueprints with C++ binding validation.

## Architecture Overview

The system consists of 4 specialized agents working in a coordinated workflow:

1. **UI Requirements Parser Agent** - Converts natural language to structured UI JSON schema
2. **Design Translation Agent** - Translates UI schema to UMG-compatible widget definitions  
3. **UE Editor Execution Agent** - Executes widget creation in UE5.6 via Python API
4. **Binding Validation Agent** - Validates C++ bindings against generated blueprints

## Quick Start

### Basic Usage

```python
import asyncio
from agents.orchestrator import AgentOrchestrator, WorkflowConfig

async def create_ui():
    # Configure the orchestrator
    config = WorkflowConfig(
        ue_tcp_host="127.0.0.1",
        ue_tcp_port=55557
    )
    
    orchestrator = AgentOrchestrator(config)
    
    # Natural language UI description
    request = """
    Create a main menu with:
    - A large "Start Game" button in the center
    - A "Settings" button below it
    - The game title at the top
    """
    
    # Execute the complete workflow
    result = await orchestrator.execute_workflow(request)
    
    if result.status == "success":
        print(f"Widget created: {result.execution_result['widget_blueprint_path']}")
    else:
        print(f"Failed: {result.errors}")

asyncio.run(create_ui())
```

### Using the MCP Server

```python
# Start the agent MCP server
python -m agents.agent_mcp_server

# Use via MCP tools
{
    "tool": "generate_ui_from_text",
    "arguments": {
        "natural_language_request": "Create a player HUD with health bar and score display",
        "cpp_header_path": "/path/to/PlayerHUD.h"
    }
}
```

## Features

### Natural Language Processing
- Extracts UI components from natural language descriptions
- Identifies layout relationships and hierarchy
- Recognizes component properties and styling requirements
- Supports complex UI scenarios with multiple components

### UMG Integration
- Creates Widget Blueprints in UE5.6
- Adds components with proper hierarchy
- Sets up event bindings and properties
- Validates component types and relationships

### C++ Binding Validation
- Parses C++ header files for UPROPERTY(meta=(BindWidget)) declarations
- Validates widget names and types against blueprints
- Provides suggestions for fixing binding mismatches
- Generates comprehensive validation reports

### Advanced Features
- **Transaction Support**: Batch operations with rollback capabilities
- **Figma Integration**: Import designs from Figma API
- **Progress Tracking**: Real-time workflow progress monitoring
- **Error Recovery**: Automatic retry and rollback mechanisms
- **Performance Monitoring**: Execution metrics and optimization

## Configuration

### Environment Variables

```bash
# UE Connection
export AGENT_UE_HOST=127.0.0.1
export AGENT_UE_PORT=55557
export AGENT_UE_PROJECT_PATH=/path/to/project

# Figma Integration
export AGENT_FIGMA_TOKEN=your_figma_token

# Logging
export AGENT_LOG_LEVEL=INFO
export AGENT_LOG_FILE=/path/to/logs/agent.log
```

### Configuration File

```yaml
# config.yaml
ue:
  tcp_host: "127.0.0.1"
  tcp_port: 55557
  project_path: "/path/to/UE/project"
  widget_path: "/Game/Widgets"
  cpp_header_paths:
    - "/path/to/Source/MyGame/UI/"

figma:
  api_token: "your_figma_token"
  cache_enabled: true

validation:
  strict_naming: true
  require_cpp_bindings: false
  validate_widget_types: true

logging:
  level: "INFO"
  file_path: "/path/to/logs/agent.log"
  console_output: true
```

## Workflow Steps

### 1. UI Requirements Parsing
- **Input**: Natural language description
- **Output**: Structured UI specification with components, layout, and interactions
- **Features**: Component type mapping, position extraction, property identification

### 2. Design Translation
- **Input**: UI specification JSON
- **Output**: UMG-compatible widget definition
- **Features**: UMG type mapping, naming conventions, hierarchy validation

### 3. UE Editor Execution
- **Input**: UMG widget definition
- **Output**: Created Widget Blueprint in UE
- **Features**: Component creation, event binding, compilation validation

### 4. Binding Validation
- **Input**: Widget blueprint path and C++ header
- **Output**: Validation report with binding status
- **Features**: C++ parsing, type checking, suggestion generation

## Examples

### Complex UI Generation

```python
request = """
Create a player HUD with:
- Health bar (progress bar) at top left
- Mana bar (blue progress bar) below health
- Score display (large text) at top right
- Minimap (square image) at bottom right
- Chat box (scrollable text area) at bottom left
- Crosshair (small image) in the center
"""

result = await orchestrator.execute_workflow(request)
```

### Figma Import

```python
from agents.tools.figma_ui_tool import FigmaUITool

async with FigmaUITool({"figma_token": "your_token"}) as figma:
    ui_spec = await figma.fetch_from_url(
        "https://www.figma.com/file/ABC123/Design-Name"
    )
    
    result = await orchestrator.execute_workflow(
        "Create widget from Figma design",
        context={"ui_specification": ui_spec}
    )
```

### Batch Operations

```python
from agents.tools.ue_python_tool import UEPythonTool

async with UEPythonTool() as ue_tool:
    commands = [
        {"command_type": "create_umg_widget_blueprint", "params": {...}},
        {"command_type": "add_widget_component", "params": {...}},
        {"command_type": "bind_widget_event", "params": {...}}
    ]
    
    results = await ue_tool.execute_batch(commands, parallel=True)
```

## Testing

Run the comprehensive test suite:

```bash
# Install test dependencies
pip install pytest pytest-asyncio

# Run all tests
pytest agents/tests/

# Run specific test categories
pytest agents/tests/test_end_to_end.py
pytest agents/tests/test_individual_agents.py
```

## Error Handling

The system includes comprehensive error handling:

- **Connection Failures**: Automatic retry with exponential backoff
- **Validation Errors**: Detailed error messages with suggestions
- **Timeout Handling**: Configurable timeouts with graceful degradation
- **Rollback Support**: Transaction-based operations with automatic rollback

## Performance

Performance optimizations include:

- **Connection Pooling**: Reuse TCP connections to UE
- **Batch Operations**: Execute multiple commands efficiently
- **Caching**: Cache Figma designs and validation results
- **Parallel Execution**: Run independent operations concurrently

## Monitoring

Built-in monitoring provides:

- **Execution Metrics**: Response times, success rates, error counts
- **Progress Tracking**: Real-time workflow progress updates
- **Performance Stats**: Connection pool status, queue sizes
- **Health Checks**: System component status monitoring

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

This project is part of the Unreal MCP integration and follows the same licensing terms.

## Support

For issues and questions:
1. Check the examples in `agents/examples/`
2. Review the test cases in `agents/tests/`
3. Consult the configuration documentation
4. Open an issue with detailed reproduction steps
