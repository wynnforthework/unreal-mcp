"""
Editor Tools for Unreal MCP.

This module provides tools for controlling the Unreal Editor viewport and other editor functionality.
"""

import logging
from typing import Dict, List, Any
from mcp.server.fastmcp import FastMCP, Context
from utils.editor.editor_operations import (
    get_actors_in_level,
    find_actors_by_name,
    spawn_actor,
    delete_actor,
    set_actor_transform,
    get_actor_properties,
    set_actor_property,
    focus_viewport,
    spawn_blueprint_actor
)

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_editor_tools(mcp: FastMCP):
    """Register editor tools with the MCP server."""
    
    @mcp.tool()
    def get_actors_in_level(ctx: Context) -> List[Dict[str, Any]]:
        """
        Get a list of all actors in the current level.
        
        Returns:
            List of actors in the current level with their properties
            
        Examples:
            actors = get_actors_in_level()
            # Print names of all actors in level
            for actor in actors:
                print(actor["name"])
        """
        return get_actors_in_level(ctx)

    @mcp.tool()
    def find_actors_by_name(ctx: Context, pattern: str) -> List[str]:
        """
        Find actors by name pattern.
        
        Args:
            pattern: Name pattern to search for (supports wildcards using *)
            
        Returns:
            List of actor names matching the pattern
            
        Examples:
            # Find all Point Light actors
            lights = find_actors_by_name("*PointLight*")
            
            # Find a specific actor
            player = find_actors_by_name("Player*")
        """
        return find_actors_by_name(ctx, pattern)
    
    @mcp.tool()
    def spawn_actor(
        ctx: Context,
        name: str,
        type: str,
        location: List[float] = None,
        rotation: List[float] = None
    ) -> Dict[str, Any]:
        """
        Create a new actor in the current level.
        
        Args:
            name: The name to give the new actor (must be unique)
            type: The type of actor to create (e.g. StaticMeshActor, PointLight)
            location: The [x, y, z] world location to spawn at
            rotation: The [pitch, yaw, roll] rotation in degrees
            
        Returns:
            Dict containing the created actor's properties
            
        Examples:
            # Spawn a point light at origin
            spawn_actor(name="MyLight", type="PointLight")
            
            # Spawn a static mesh at a specific location
            spawn_actor(name="MyCube", type="StaticMeshActor", 
                       location=[100, 200, 50], 
                       rotation=[0, 45, 0])
        """
        return spawn_actor(ctx, name, type, location, rotation)
    
    @mcp.tool()
    def delete_actor(ctx: Context, name: str) -> Dict[str, Any]:
        """
        Delete an actor by name.
        
        Args:
            name: Name of the actor to delete
            
        Returns:
            Dict containing response information
            
        Examples:
            # Delete an actor named "MyCube"
            delete_actor(name="MyCube")
        """
        return delete_actor(ctx, name)
    
    @mcp.tool()
    def set_actor_transform(
        ctx: Context,
        name: str,
        location: List[float] = None,
        rotation: List[float] = None,
        scale: List[float] = None
    ) -> Dict[str, Any]:
        """
        Set the transform of an actor.
        
        Args:
            name: Name of the actor
            location: Optional [X, Y, Z] position
            rotation: Optional [Pitch, Yaw, Roll] rotation in degrees
            scale: Optional [X, Y, Z] scale
            
        Returns:
            Dict containing response information
            
        Examples:
            # Move an actor named "MyCube" to a new position
            set_actor_transform(name="MyCube", location=[100, 200, 50])
            
            # Rotate an actor named "MyCube" 45 degrees around Z axis
            set_actor_transform(name="MyCube", rotation=[0, 0, 45])
            
            # Scale an actor named "MyCube" to be twice as big
            set_actor_transform(name="MyCube", scale=[2.0, 2.0, 2.0])
            
            # Move, rotate, and scale an actor all at once
            set_actor_transform(
                name="MyCube", 
                location=[100, 200, 50],
                rotation=[0, 0, 45],
                scale=[2.0, 2.0, 2.0]
            )
        """
        return set_actor_transform(ctx, name, location, rotation, scale)
    
    @mcp.tool()
    def get_actor_properties(ctx: Context, name: str) -> Dict[str, Any]:
        """
        Get all properties of an actor.
        
        Args:
            name: Name of the actor
            
        Returns:
            Dict containing actor properties
            
        Examples:
            # Get properties of an actor named "MyCube"
            props = get_actor_properties(name="MyCube")
            
            # Print location
            print(props["transform"]["location"])
        """
        return get_actor_properties(ctx, name)

    @mcp.tool()
    def set_actor_property(
        ctx: Context,
        name: str,
        property_name: str,
        property_value: Any
    ) -> Dict[str, Any]:
        """
        Set a property on an actor.
        
        Args:
            name: Name of the actor
            property_name: Name of the property to set
            property_value: Value to set the property to
            
        Returns:
            Dict containing response information
            
        Examples:
            # Change the color of a light
            set_actor_property(
                name="MyPointLight",
                property_name="LightColor",
                property_value=[255, 0, 0, 255]  # RGBA
            )
            
            # Change the mobility of an actor
            set_actor_property(
                name="MyCube",
                property_name="Mobility",
                property_value="Movable"  # "Static", "Stationary", or "Movable"
            )
        """
        return set_actor_property(ctx, name, property_name, property_value)

    # @mcp.tool() commented out because it's buggy
    def focus_viewport(
        ctx: Context,
        target: str = None,
        location: List[float] = None,
        distance: float = 1000.0,
        orientation: List[float] = None
    ) -> Dict[str, Any]:
        """
        Focus the viewport on a specific actor or location.
        
        Args:
            target: Name of the actor to focus on (if provided, location is ignored)
            location: [X, Y, Z] coordinates to focus on (used if target is None)
            distance: Distance from the target/location
            orientation: Optional [Pitch, Yaw, Roll] for the viewport camera
            
        Returns:
            Response from Unreal Engine
            
        Examples:
            # Focus on an actor named "MyCube"
            focus_viewport(target="MyCube")
            
            # Focus on a specific location
            focus_viewport(location=[100, 200, 50])
            
            # Focus on an actor from a specific orientation
            focus_viewport(target="MyCube", orientation=[45, 0, 0])
        """
        return focus_viewport(ctx, target, location, distance, orientation)

    @mcp.tool()
    def spawn_blueprint_actor(
        ctx: Context,
        blueprint_name: str,
        actor_name: str,
        location: List[float] = None,
        rotation: List[float] = None
    ) -> Dict[str, Any]:
        """
        Spawn an actor from a Blueprint.
        
        Args:
            blueprint_name: Name of the Blueprint to spawn from
            actor_name: Name to give the spawned actor
            location: The [x, y, z] world location to spawn at
            rotation: The [pitch, yaw, roll] rotation in degrees
            
        Returns:
            Dict containing the spawned actor's properties
            
        Examples:
            # Spawn a blueprint actor at origin
            spawn_blueprint_actor(
                blueprint_name="/Game/Blueprints/MyActor",
                actor_name="MyActor_1"
            )
            
            # Spawn a blueprint actor at a specific location and rotation
            spawn_blueprint_actor(
                blueprint_name="/Game/Blueprints/MyActor",
                actor_name="MyActor_2",
                location=[100, 200, 50],
                rotation=[0, 45, 0]
            )
        """
        return spawn_blueprint_actor(ctx, blueprint_name, actor_name, location, rotation)

    logger.info("Editor tools registered successfully")
