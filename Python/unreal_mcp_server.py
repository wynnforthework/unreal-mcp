"""
Unreal Engine MCP Server

A simple MCP server for interacting with Unreal Engine.
"""

import logging
import socket
import sys
import json
from contextlib import asynccontextmanager
from typing import AsyncIterator, Dict, Any, Optional
from mcp.server.fastmcp import FastMCP

# Configure logging with more detailed format
logging.basicConfig(
    level=logging.DEBUG,  # Change to DEBUG level for more details
    format='%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s',
    handlers=[
        logging.FileHandler('unreal_mcp.log'),
        # logging.StreamHandler(sys.stdout) # Remove this handler to unexpected non-whitespace characters in JSON
    ]
)
logger = logging.getLogger("UnrealMCP")

# Configuration
UNREAL_HOST = "127.0.0.1"
UNREAL_PORT = 55557

class UnrealConnection:
    """Connection to an Unreal Engine instance."""
    
    def __init__(self):
        """Initialize the connection."""
        self.socket = None
        self.connected = False
    
    def connect(self) -> bool:
        """Connect to the Unreal Engine instance."""
        try:
            # Close any existing socket
            if self.socket:
                try:
                    self.socket.close()
                except:
                    pass
                self.socket = None
            
            logger.info(f"Connecting to Unreal at {UNREAL_HOST}:{UNREAL_PORT}...")
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(5)  # 5 second timeout
            
            # Set socket options for better stability
            self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
            
            # Set larger buffer sizes
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)
            
            self.socket.connect((UNREAL_HOST, UNREAL_PORT))
            self.connected = True
            logger.info("Connected to Unreal Engine")
            return True
            
        except Exception as e:
            logger.error(f"Failed to connect to Unreal: {e}")
            self.connected = False
            return False
    
    def disconnect(self):
        """Disconnect from the Unreal Engine instance."""
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
        self.socket = None
        self.connected = False

    def receive_full_response(self, sock, buffer_size=4096) -> bytes:
        """Receive a complete response from Unreal, handling chunked data."""
        chunks = []
        sock.settimeout(5)  # 5 second timeout
        try:
            while True:
                chunk = sock.recv(buffer_size)
                if not chunk:
                    if not chunks:
                        raise Exception("Connection closed before receiving data")
                    break
                chunks.append(chunk)
                
                # Process the data received so far
                data = b''.join(chunks)
                decoded_data = data.decode('utf-8')
                
                # Try to parse as JSON to check if complete
                try:
                    json.loads(decoded_data)
                    logger.info(f"Received complete response ({len(data)} bytes)")
                    return data
                except json.JSONDecodeError:
                    # Not complete JSON yet, continue reading
                    logger.debug(f"Received partial response, waiting for more data...")
                    continue
                except Exception as e:
                    logger.warning(f"Error processing response chunk: {str(e)}")
                    continue
        except socket.timeout:
            logger.warning("Socket timeout during receive")
            if chunks:
                # If we have some data already, try to use it
                data = b''.join(chunks)
                try:
                    json.loads(data.decode('utf-8'))
                    logger.info(f"Using partial response after timeout ({len(data)} bytes)")
                    return data
                except:
                    pass
            raise Exception("Timeout receiving Unreal response")
        except Exception as e:
            logger.error(f"Error during receive: {str(e)}")
            raise
    
    def send_command(self, command: str, params: Dict[str, Any] = None) -> Optional[Dict[str, Any]]:
        """Send a command to Unreal Engine and get the response."""
        # Always reconnect for each command, since Unreal closes the connection after each command
        # This is different from Unity which keeps connections alive
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
            self.connected = False
        
        if not self.connect():
            logger.error("Failed to connect to Unreal Engine for command")
            return None
        
        try:
            # Match Unity's command format exactly
            command_obj = {
                "type": command,  # Use "type" instead of "command"
                "params": params or {}  # Use Unity's params or {} pattern
            }
            
            # Send without newline, exactly like Unity
            command_json = json.dumps(command_obj)
            logger.info(f"Sending command: {command_json}")
            self.socket.sendall(command_json.encode('utf-8'))
            
            # Read response using improved handler
            response_data = self.receive_full_response(self.socket)
            response = json.loads(response_data.decode('utf-8'))
            
            # Log complete response for debugging
            logger.info(f"Complete response from Unreal: {response}")
            
            # Check for both error formats: {"status": "error", ...} and {"success": false, ...}
            if response.get("status") == "error":
                error_message = response.get("error") or response.get("message", "Unknown Unreal error")
                logger.error(f"Unreal error (status=error): {error_message}")
                # We want to preserve the original error structure but ensure error is accessible
                if "error" not in response:
                    response["error"] = error_message
            elif response.get("success") is False:
                # This format uses {"success": false, "error": "message"} or {"success": false, "message": "message"}
                error_message = response.get("error") or response.get("message", "Unknown Unreal error")
                logger.error(f"Unreal error (success=false): {error_message}")
                # Convert to the standard format expected by higher layers
                response = {
                    "status": "error",
                    "error": error_message
                }
            
            # Always close the connection after command is complete
            # since Unreal will close it on its side anyway
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
            self.connected = False
            
            return response
            
        except Exception as e:
            logger.error(f"Error sending command: {e}")
            # Always reset connection state on any error
            self.connected = False
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
            return {
                "status": "error",
                "error": str(e)
            }

# Global connection state
_unreal_connection: UnrealConnection = None

def get_unreal_connection() -> Optional[UnrealConnection]:
    """Get the connection to Unreal Engine."""
    global _unreal_connection
    try:
        if _unreal_connection is None:
            _unreal_connection = UnrealConnection()
            if not _unreal_connection.connect():
                logger.warning("Could not connect to Unreal Engine")
                _unreal_connection = None
        else:
            # Verify connection is still valid with a ping-like test
            try:
                # Simple test by sending an empty buffer to check if socket is still connected
                _unreal_connection.socket.sendall(b'\x00')
                logger.debug("Connection verified with ping test")
            except Exception as e:
                logger.warning(f"Existing connection failed: {e}")
                _unreal_connection.disconnect()
                _unreal_connection = None
                # Try to reconnect
                _unreal_connection = UnrealConnection()
                if not _unreal_connection.connect():
                    logger.warning("Could not reconnect to Unreal Engine")
                    _unreal_connection = None
                else:
                    logger.info("Successfully reconnected to Unreal Engine")
        
        return _unreal_connection
    except Exception as e:
        logger.error(f"Error getting Unreal connection: {e}")
        return None

@asynccontextmanager
async def server_lifespan(server: FastMCP) -> AsyncIterator[Dict[str, Any]]:
    """Handle server startup and shutdown."""
    global _unreal_connection
    logger.info("UnrealMCP server starting up")
    try:
        _unreal_connection = get_unreal_connection()
        if _unreal_connection:
            logger.info("Connected to Unreal Engine on startup")
        else:
            logger.warning("Could not connect to Unreal Engine on startup")
    except Exception as e:
        logger.error(f"Error connecting to Unreal Engine on startup: {e}")
        _unreal_connection = None
    
    try:
        yield {}
    finally:
        if _unreal_connection:
            _unreal_connection.disconnect()
            _unreal_connection = None
        logger.info("Unreal MCP server shut down")

# Initialize server
mcp = FastMCP(
    "UnrealMCP",
    description="Unreal Engine integration via Model Context Protocol",
    lifespan=server_lifespan
)

# Import and register tools
from tools.editor_tools import register_editor_tools
from tools.blueprint_tools import register_blueprint_tools
from tools.node_tools import register_blueprint_node_tools
from tools.project_tools import register_project_tools
from tools.umg_tools import register_umg_tools
from tools.datatable_tools import register_datatable_tools

# Register tools
register_editor_tools(mcp)
register_blueprint_tools(mcp)
register_blueprint_node_tools(mcp)
register_project_tools(mcp)
register_umg_tools(mcp)
register_datatable_tools(mcp)

@mcp.prompt()
def info():
    """Information about available Unreal MCP tools and best practices."""
    return """
    # Unreal MCP Server Tools and Best Practices
    
    ## UMG (Widget Blueprint) Tools
    
    - **create_umg_widget_blueprint(widget_name, parent_class="UserWidget", path="/Game/Widgets")**
      
      Create a new UMG Widget Blueprint.
      
      Args:
        - widget_name: Name of the widget blueprint to create
        - parent_class: Parent class for the widget (default: UserWidget)
        - path: Content browser path where the widget should be created
      
      Returns: Dict containing success status and widget path
      
      Example:
        create_umg_widget_blueprint(widget_name="MyWidget")
    
    - **bind_widget_component_event(widget_name, widget_component_name, event_name, function_name="")**
      
      Bind an event on a widget component to a function.
      
      Args:
        - widget_name: Name of the target Widget Blueprint
        - widget_component_name: Name of the widget component (button, etc.)
        - event_name: Name of the event to bind (OnClicked, etc.)
        - function_name: Name of the function to create/bind to (optional)
      
      Returns: Dict containing success status and binding information
      
      Example:
        bind_widget_component_event(widget_name="LoginScreen", widget_component_name="LoginButton", event_name="OnClicked")
    
    - **add_widget_to_viewport(widget_name, z_order=0)**
      
      Add a Widget Blueprint instance to the viewport.
      
      Args:
        - widget_name: Name of the Widget Blueprint to add
        - z_order: Z-order for the widget (higher numbers appear on top)
      
      Returns: Dict containing success status and widget instance information
      
      Example:
        add_widget_to_viewport(widget_name="MainMenu")
    
    - **set_text_block_widget_component_binding(widget_name, text_block_name, binding_property, binding_type="Text")**
      
      Set up a property binding for a Text Block widget.
      
      Args:
        - widget_name: Name of the target Widget Blueprint
        - text_block_name: Name of the Text Block to bind
        - binding_property: Name of the property to bind to
        - binding_type: Type of binding (Text, Visibility, etc.)
      
      Returns: Dict containing success status and binding information
      
      Example:
        set_text_block_widget_component_binding(widget_name="PlayerHUD", text_block_name="ScoreText", binding_property="CurrentScore")
    
    - **add_child_widget_component_to_parent(widget_name, parent_component_name, child_component_name, create_parent_if_missing=False, parent_component_type="Border", parent_position=[0.0, 0.0], parent_size=[300.0, 200.0])**
      
      Add a widget component as a child to another component. Can create the parent if missing.
      
      Args:
        - widget_name: Name of the target Widget Blueprint
        - parent_component_name: Name of the parent component
        - child_component_name: Name of the child component to add to the parent
        - create_parent_if_missing: Whether to create the parent component if it doesn't exist
        - parent_component_type: Type of parent component to create if needed (e.g., "Border", "VerticalBox")
        - parent_position: [X, Y] position of the parent component if created
        - parent_size: [Width, Height] of the parent component if created
      
      Returns: Dict containing success status and component relationship information
      
      Example:
        add_child_widget_component_to_parent(widget_name="MyWidget", parent_component_name="ContentBorder", child_component_name="HeaderText")
    
    - **create_parent_and_child_widget_components(widget_name, parent_component_name, child_component_name, parent_component_type="Border", child_component_type="TextBlock", parent_position=[0.0, 0.0], parent_size=[300.0, 200.0], child_attributes=None)**
      
      Create a new parent widget component with a new child component (one parent, one child).
      
      Args:
        - widget_name: Name of the target Widget Blueprint
        - parent_component_name: Name for the new parent component
        - child_component_name: Name for the new child component
        - parent_component_type: Type of parent component to create (e.g., "Border", "VerticalBox")
        - child_component_type: Type of child component to create (e.g., "TextBlock", "Button")
        - parent_position: [X, Y] position of the parent component
        - parent_size: [Width, Height] of the parent component
        - child_attributes: Additional attributes for the child component (content, colors, etc.)
      
      Returns: Dict containing success status and component creation information
      
      Example:
        create_parent_and_child_widget_components(widget_name="MyWidget", parent_component_name="HeaderBorder", child_component_name="TitleText", parent_component_type="Border", child_component_type="TextBlock", parent_position=[50.0, 50.0], parent_size=[400.0, 100.0], child_attributes={"text": "Welcome to My Game", "font_size": 24})
    
    - **check_widget_component_exists(widget_name, component_name)**
      
      Check if a component exists in the specified widget blueprint.
      
      Args:
        - widget_name: Name of the target Widget Blueprint
        - component_name: Name of the component to check
      
      Returns: Dict containing existence status of the component
      
      Example:
        check_widget_component_exists(widget_name="MyWidget", component_name="HeaderText")
    
    - **set_widget_component_placement(widget_name, component_name, position=None, size=None, alignment=None)**
      
      Change the placement (position/size) of a widget component.
      
      Args:
        - widget_name: Name of the target Widget Blueprint
        - component_name: Name of the component to modify
        - position: Optional [X, Y] new position in the canvas panel
        - size: Optional [Width, Height] new size for the component
        - alignment: Optional [X, Y] alignment values (0.0 to 1.0)
      
      Returns: Dict containing success status and updated placement information
      
      Example:
        set_widget_component_placement(widget_name="MainMenu", component_name="TitleText", position=[350.0, 75.0])
    
    - **get_widget_container_component_dimensions(widget_name, container_name="CanvasPanel_0")**
      
      Get the dimensions of a container widget in a UMG Widget Blueprint.
      
      Args:
        - widget_name: Name of the target Widget Blueprint
        - container_name: Name of the container widget (defaults to "CanvasPanel_0" for the root canvas panel)
      
      Returns: Dict containing the container dimensions and position
      
      Example:
        get_widget_container_component_dimensions(widget_name="MainMenu")
    
    - **add_widget_component_to_widget(widget_name, component_name, component_type, position=None, size=None, **kwargs)**
      
      Unified function to add any type of widget component to a UMG Widget Blueprint.
      
      Args:
        - widget_name: Name of the target Widget Blueprint
        - component_name: Name to give the new component
        - component_type: Type of component to add (e.g., "TextBlock", "Button", etc.)
        - position: Optional [X, Y] position in the canvas panel
        - size: Optional [Width, Height] of the component
        - **kwargs: Additional parameters specific to the component type
      
      Returns: Dict containing success status and component properties
      
      Example:
        add_widget_component_to_widget(widget_name="MyWidget", component_name="HeaderText", component_type="TextBlock", position=[100, 50], size=[200, 50], text="Hello World", font_size=24)
    
    - **set_widget_component_property(widget_name, component_name, **kwargs)**
      
      Set one or more properties on a specific component within a UMG Widget Blueprint.
      
      Args:
        - widget_name: Name of the target Widget Blueprint
        - component_name: Name of the component to modify
        - **kwargs: Properties to set (as keyword arguments or a dict)
      
      Returns: Dict containing success status and property update info
      
      Example:
        set_widget_component_property("MyWidget", "MyTextBlock", Text="Red Text", ColorAndOpacity={"SpecifiedColor": {"R": 1.0, "G": 0.0, "B": 0.0, "A": 1.0}})
    
    - **get_widget_component_layout(widget_name)**
      
      Get hierarchical layout information for all components within a UMG Widget Blueprint.
      
      Args:
        - widget_name: Name of the target Widget Blueprint (e.g., "WBP_MainMenu", "/Game/UI/MyWidget")
      
      Returns: Dict containing:
        - success (bool): True if the operation succeeded.
        - message (str): Status message.
        - hierarchy (dict): Root component with the following structure:
            - name (str): Component name.
            - type (str): Component class name (e.g., "TextBlock", "Button").
            - slot_properties (dict): Layout properties based on the slot type.
            - children (list[dict]): List of child components with the same structure.
      
      Example:
        layout = get_widget_component_layout(widget_name="WBP_PricingPage")
        if layout.get("success"):
            hierarchy = layout.get("hierarchy", {})
            print(f"Root component: {hierarchy.get('name')} ({hierarchy.get('type')})")
    
    ## Editor Tools
    ### Viewport and Screenshots
    - `focus_viewport(target, location, distance, orientation)` - Focus viewport
    - `take_screenshot(filename, show_ui, resolution)` - Capture screenshots

    ### Actor Management
    - **get_actors_in_level()**
      
      Get a list of all actors in the current level.
      
      Returns: List of actors in the current level with their properties.
      
      Example:
        actors = get_actors_in_level()
        for actor in actors:
            print(actor["name"])
    
    - **find_actors_by_name(pattern)**
      
      Find actors by name pattern (supports wildcards using *).
      
      Args:
        - pattern (str): Name pattern to search for
      
      Returns: List of actor names matching the pattern.
      
      Example:
        lights = find_actors_by_name("*PointLight*")
        player = find_actors_by_name("Player*")
    
    - **spawn_actor(name, type, location=None, rotation=None)**
      
      Create a new basic Unreal Engine actor in the current level (built-in types only).
      
      Args:
        - name (str): The name to give the new actor (must be unique)
        - type (str): The type of built-in actor to create (StaticMeshActor, PointLight, SpotLight, DirectionalLight, CameraActor)
        - location (list): The [x, y, z] world location to spawn at
        - rotation (list): The [pitch, yaw, roll] rotation in degrees
      
      Returns: Dict containing the created actor's properties.
      
      Example:
        spawn_actor(name="MyLight", type="PointLight")
        spawn_actor(name="MyCube", type="StaticMeshActor", location=[100, 200, 50], rotation=[0, 45, 0])
    
    - **delete_actor(name)**
      
      Delete an actor by name.
      
      Args:
        - name (str): Name of the actor to delete
      
      Returns: Dict containing response information.
      
      Example:
        delete_actor(name="MyCube")
    
    - **set_actor_transform(name, location=None, rotation=None, scale=None)**
      
      Set the transform of an actor.
      
      Args:
        - name (str): Name of the actor
        - location (list): Optional [X, Y, Z] position
        - rotation (list): Optional [Pitch, Yaw, Roll] rotation in degrees
        - scale (list): Optional [X, Y, Z] scale
      
      Returns: Dict containing response information.
      
      Example:
        set_actor_transform(name="MyCube", location=[100, 200, 50])
        set_actor_transform(name="MyCube", rotation=[0, 0, 45])
        set_actor_transform(name="MyCube", scale=[2.0, 2.0, 2.0])
        set_actor_transform(name="MyCube", location=[100, 200, 50], rotation=[0, 0, 45], scale=[2.0, 2.0, 2.0])
    
    - **get_actor_properties(name)**
      
      Get all properties of an actor.
      
      Args:
        - name (str): Name of the actor
      
      Returns: Dict containing actor properties.
      
      Example:
        props = get_actor_properties(name="MyCube")
        print(props["transform"]["location"])
    
    - **set_actor_property(name, property_name, property_value)**
      
      Set a property on an actor.
      
      Args:
        - name (str): Name of the actor
        - property_name (str): Name of the property to set
        - property_value: Value to set the property to (various types)
      
      Returns: Dict containing response information.
      
      Example:
        set_actor_property(name="MyPointLight", property_name="LightColor", property_value=[255, 0, 0, 255])
        set_actor_property(name="MyCube", property_name="Mobility", property_value="Movable")
        set_actor_property(name="MyCube", property_name="bHidden", property_value=True)
        set_actor_property(name="PointLightTest", property_name="Intensity", property_value=5000.0)

    ## Blueprint Management
    - `create_blueprint(name, parent_class)` - Create new Blueprint classes
    - `add_component_to_blueprint(blueprint_name, component_type, component_name)` - Add components
    - `set_static_mesh_properties(blueprint_name, component_name, static_mesh)` - Configure meshes
    - `set_physics_properties(blueprint_name, component_name)` - Configure physics
    - `compile_blueprint(blueprint_name)` - Compile Blueprint changes
    - `set_blueprint_property(blueprint_name, property_name, property_value)` - Set properties
    - `set_pawn_properties(blueprint_name)` - Configure Pawn settings
    - `spawn_blueprint_actor(blueprint_name, actor_name)` - Spawn Blueprint actors
    
    ## Blueprint Node Management
    - `add_blueprint_event_node(blueprint_name, event_type)` - Add event nodes
    - `add_blueprint_input_action_node(blueprint_name, action_name)` - Add input nodes
    - `add_blueprint_function_node(blueprint_name, target, function_name)` - Add function nodes
    - `connect_blueprint_nodes(blueprint_name, source_node_id, source_pin, target_node_id, target_pin)` - Connect nodes
    - `add_blueprint_variable(blueprint_name, variable_name, variable_type)` - Add variables
    - `add_blueprint_get_self_component_reference(blueprint_name, component_name)` - Add component refs
    - `add_blueprint_self_reference(blueprint_name)` - Add self references
    - `find_blueprint_nodes(blueprint_name, node_type, event_type)` - Find nodes
    
    ## Project Tools
    - `create_input_mapping(action_name, key, input_type)` - Create input mappings
    
    ## Best Practices
    
    ### UMG Widget Development
    - Create widgets with descriptive names that reflect their purpose
    - Use consistent naming conventions for widget components
    - Organize widget hierarchy logically
    - Set appropriate anchors and alignment for responsive layouts
    - Use property bindings for dynamic updates instead of direct setting
    - Handle widget events appropriately with meaningful function names
    - Clean up widgets when no longer needed
    - Test widget layouts at different resolutions
    
    ### Editor and Actor Management
    - Use unique names for actors to avoid conflicts
    - Clean up temporary actors
    - Validate transforms before applying
    - Check actor existence before modifications
    - Take regular viewport screenshots during development
    - Keep the viewport focused on relevant actors during operations
    
    ### Blueprint Development
    - Compile Blueprints after changes
    - Use meaningful names for variables and functions
    - Organize nodes logically
    - Test functionality in isolation
    - Consider performance implications
    - Document complex setups
    
    ### Error Handling
    - Check command responses for success
    - Handle errors gracefully
    - Log important operations
    - Validate parameters
    - Clean up resources on errors
    """

# ... rest of the file remains unchanged ...