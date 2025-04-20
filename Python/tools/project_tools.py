"""
Project Tools for Unreal MCP.

This module provides tools for managing project-wide settings and configuration.
"""

import logging
from typing import Dict, Any
from mcp.server.fastmcp import FastMCP, Context

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_project_tools(mcp: FastMCP):
    """Register project tools with the MCP server."""
    
    @mcp.tool()
    def create_input_mapping(
        ctx: Context,
        action_name: str,
        key: str,
        input_type: str = "Action"
    ) -> Dict[str, Any]:
        """
        Create an input mapping for the project.
        
        Args:
            action_name: Name of the input action
            key: Key to bind (SpaceBar, LeftMouseButton, etc.)
            input_type: Type of input mapping (Action or Axis)
            
        Returns:
            Response indicating success or failure
        
        Example:
            create_input_mapping(action_name="Jump", key="SpaceBar")
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "action_name": action_name,
                "key": key,
                "input_type": input_type
            }
            
            logger.info(f"Creating input mapping '{action_name}' with key '{key}'")
            response = unreal.send_command("create_input_mapping", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Input mapping creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error creating input mapping: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def create_folder(
        ctx: Context,
        folder_path: str
    ) -> Dict[str, Any]:
        """
        Create a new folder in the Unreal project.

        This tool can create both content folders (which will appear in the content browser)
        and regular project folders.

        Args:
            folder_path: Path to create, relative to project root.
                       Use "Content/..." prefix for content browser folders.

        Returns:
            Dictionary with the creation status and folder path
            
        Examples:
            # Create a content browser folder
            create_folder(folder_path="Content/MyGameContent")
            
            # Create a regular project folder
            create_folder(folder_path="Intermediate/MyTools")
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "folder_path": folder_path
            }
            
            logger.info(f"Creating folder: {folder_path}")
            response = unreal.send_command("create_folder", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Folder creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error creating folder: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    # For backward compatibility
    @mcp.tool()
    def create_project_folder(
        ctx: Context,
        folder_path: str
    ) -> Dict[str, Any]:
        """
        [DEPRECATED] Please use create_folder instead.
        Create a new folder in the Unreal project.

        Args:
            folder_path: Path to create, relative to project root.
                       Use "Content/..." prefix for content browser folders.

        Returns:
            Dictionary with the creation status and folder path
        """
        logger.warning("create_project_folder is deprecated, please use create_folder instead")
        return create_folder(ctx, folder_path)

    logger.info("Project tools registered successfully")