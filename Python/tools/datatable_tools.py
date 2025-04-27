from __future__ import annotations
from typing import Dict, List, Any, Optional
import sys
import os

# Add the parent directory to sys.path to enable absolute imports
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from utils.datatable.datatable_utils import (
    create_datatable_impl,
    get_datatable_rows_impl,
    delete_datatable_row_impl,
    get_datatable_row_names_impl,
    add_rows_to_datatable_impl,
    update_rows_in_datatable_impl,
    delete_datatable_rows_impl
)

def register_datatable_tools(mcp: 'FastMCP'):
    """Register DataTable-related tools with the MCP server."""
    
    @mcp.tool()
    def create_datatable(
        datatable_name: str,
        row_struct_name: str,
        path: str = "/Game/Data",
        description: str = ""
    ) -> Dict[str, Any]:
        """Create a new DataTable asset.
        
        Args:
            datatable_name: Name of the DataTable to create
            row_struct_name: Name of the struct to use for rows
            path: Path where to create the DataTable
            description: Optional description for the DataTable
            
        Returns:
            Dict containing success status and created asset info
        """
        return create_datatable_impl(datatable_name, row_struct_name, path, description)
    
    @mcp.tool()
    def get_datatable_rows(
        datatable_name: str,
        row_names: Optional[List[str]] = None
    ) -> Dict[str, Any]:
        """Get rows from a DataTable.
        
        Args:
            datatable_name: Name of the target DataTable
            row_names: Optional list of specific row names to retrieve
            
        Returns:
            Dict containing the requested rows
        """
        return get_datatable_rows_impl(datatable_name, row_names)
    
    @mcp.tool()
    def delete_datatable_row(
        datatable_name: str,
        row_name: str
    ) -> Dict[str, Any]:
        """Delete a row from a DataTable.
        
        Args:
            datatable_name: Name of the target DataTable
            row_name: Name of the row to delete
            
        Returns:
            Dict containing success status and updated DataTable info
        """
        return delete_datatable_row_impl(datatable_name, row_name)
    
    @mcp.tool()
    def get_datatable_row_names(
        datatable_name: str
    ) -> Dict[str, Any]:
        """Get all row names and struct field names from a DataTable.
        
        Args:
            datatable_name: Name of the target DataTable
        Returns:
            Dict containing:
                - 'row_names': list of row names (str)
                - 'field_names': list of struct field names (str)
        """
        return get_datatable_row_names_impl(datatable_name)
    
    @mcp.tool()
    def add_rows_to_datatable(
        datatable_name: str,
        rows: list[dict]
    ) -> Dict[str, Any]:
        """Add multiple rows to an existing DataTable.
        Args:
            datatable_name: Name of the target DataTable
            rows: List of dicts, each with 'row_name' and 'row_data'
        Returns:
            Dict containing success status and list of added row names
        """
        return add_rows_to_datatable_impl(datatable_name, rows)
    
    @mcp.tool()
    def update_rows_in_datatable(
        datatable_name: str,
        rows: list[dict]
    ) -> Dict[str, Any]:
        """Update multiple rows in an existing DataTable.
        Args:
            datatable_name: Name of the target DataTable
            rows: List of dicts, each with 'row_name' and 'row_data'
        Returns:
            Dict containing success status and list of updated/failed row names
        """
        return update_rows_in_datatable_impl(datatable_name, rows)
    
    @mcp.tool()
    def delete_datatable_rows(
        datatable_name: str,
        row_names: List[str]
    ) -> Dict[str, Any]:
        """Delete multiple rows from a DataTable.
        Args:
            datatable_name: Name of the target DataTable
            row_names: List of row names to delete
        Returns:
            Dict containing success status and updated DataTable info
        """
        return delete_datatable_rows_impl(datatable_name, row_names) 