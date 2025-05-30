from mcp.server.fastmcp import FastMCP
from node_tools.node_tools import register_blueprint_node_tools

mcp = FastMCP(
    "nodeMCP",
    description="Blueprint Node tools for Unreal via MCP"
)

register_blueprint_node_tools(mcp)

if __name__ == "__main__":
    mcp.run(transport='stdio')

"""
## Blueprint Node Management

### Important Limitation: Loop Nodes (ForEach, ForLoop, etc.)

> **Note:**
> The standard Blueprint loop nodes such as **ForEach** and **ForLoop** are implemented as Blueprint Macros, not as native K2 nodes. Due to Unreal Engine's public API limitations, these macro nodes **cannot be added programmatically** via the MCP tool or any C++ plugin. Only native K2 nodes (such as Branch, Sequence, etc.) can be added programmatically.
> If you need loop logic in your Blueprint, please provide a comment for user in place of the loop node. This is a known limitation of Unreal Engine and is not specific to this plugin.

- **add_blueprint_event_node(blueprint_name, event_name, node_position=None)**
  
  Add an event node to a Blueprint's event graph.
  
  Args:
    - blueprint_name (str): Name of the target Blueprint
    - event_name (str): Name of the event. Use 'Receive' prefix for standard events (e.g., 'ReceiveBeginPlay', 'ReceiveTick')
    - node_position (list): Optional [X, Y] position in the graph
  
  Returns: Response containing the node ID and success status.

- **add_blueprint_input_action_node(blueprint_name, action_name, node_position=None)**
  
  Add an input action event node to a Blueprint's event graph.
  
  Args:
    - blueprint_name (str): Name of the target Blueprint
    - action_name (str): Name of the input action to respond to
    - node_position (list): Optional [X, Y] position in the graph
  
  Returns: Response containing the node ID and success status.

- **add_blueprint_function_node(blueprint_name, target, function_name, params=None, node_position=None)**
  
  Add a function call node to a Blueprint's event graph.
  
  Args:
    - blueprint_name (str): Name of the target Blueprint
    - target (str): Target object for the function (component name or self)
    - function_name (str): Name of the function to call
    - params (dict): Optional parameters to set on the function node
    - node_position (list): Optional [X, Y] position in the graph
  
  Returns: Response containing the node ID and success status.

- **connect_blueprint_nodes(blueprint_name, source_node_id, source_pin, target_node_id, target_pin)**
  
  Connect two nodes in a Blueprint's event graph. You must determine the correct node IDs and pins to connect.
  
  Args:
    - blueprint_name (str): Name of the target Blueprint
    - source_node_id (str): ID of the source node
    - source_pin (str): Name of the output pin on the source node
    - target_node_id (str): ID of the target node
    - target_pin (str): Name of the input pin on the target node
  
  Returns: Response indicating success or failure.

- **add_blueprint_get_self_component_reference(blueprint_name, component_name, node_position=None)**
  
  Add a node that gets a reference to a component owned by the current Blueprint (like dragging a component from the Components panel).
  
  Args:
    - blueprint_name (str): Name of the target Blueprint
    - component_name (str): Name of the component to get a reference to
    - node_position (list): Optional [X, Y] position in the graph
  
  Returns: Response containing the node ID and success status.

- **add_blueprint_self_reference(blueprint_name, node_position=None)**
  
  Add a 'Get Self' node to a Blueprint's event graph that returns a reference to this actor.
  
  Args:
    - blueprint_name (str): Name of the target Blueprint
    - node_position (list): Optional [X, Y] position in the graph
  
  Returns: Response containing the node ID and success status.

- **find_blueprint_nodes(blueprint_name, node_type=None, event_type=None)**
  
  Find nodes in a Blueprint's event graph.
  
  Args:
    - blueprint_name (str): Name of the target Blueprint
    - node_type (str): Optional type of node to find (Event, Function, Variable, etc.)
    - event_type (str): Optional specific event type to find (BeginPlay, Tick, etc.)
  
  Returns: Response containing array of found node IDs and success status.
""" 