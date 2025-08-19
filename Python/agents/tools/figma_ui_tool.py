"""
Figma/AI UI Generation HTTP Tool

Implements an HTTP interface tool that can integrate with Figma API or AI-generated UI designs.
Fetches design specifications and converts them to the UI JSON schema format used by the agent chain.
"""

import asyncio
import aiohttp
import json
import base64
from typing import Dict, Any, List, Optional, Tuple
from dataclasses import dataclass
from urllib.parse import urljoin
import logging


@dataclass
class FigmaNode:
    """Represents a Figma design node"""
    id: str
    name: str
    type: str
    position: Dict[str, float]
    size: Dict[str, float]
    properties: Dict[str, Any]
    children: List['FigmaNode']
    
    def to_ui_component(self) -> Dict[str, Any]:
        """Convert Figma node to UI component format"""
        return {
            "id": self.id,
            "type": self._map_figma_type_to_ui_type(),
            "label": self.name,
            "description": f"Figma {self.type}: {self.name}",
            "position": self.position,
            "size": self.size,
            "properties": self._extract_ui_properties(),
            "children": [child.id for child in self.children]
        }
    
    def _map_figma_type_to_ui_type(self) -> str:
        """Map Figma node type to UI component type"""
        type_mapping = {
            "RECTANGLE": "Button" if "button" in self.name.lower() else "Panel",
            "TEXT": "TextBlock",
            "FRAME": "Panel",
            "GROUP": "Panel",
            "COMPONENT": "Button",
            "INSTANCE": "Button",
            "VECTOR": "Image",
            "ELLIPSE": "Image",
            "LINE": "Border"
        }
        return type_mapping.get(self.type, "Panel")
    
    def _extract_ui_properties(self) -> Dict[str, Any]:
        """Extract UI properties from Figma properties"""
        ui_props = {}
        
        # Extract text properties
        if "characters" in self.properties:
            ui_props["text"] = self.properties["characters"]
        
        # Extract color properties
        if "fills" in self.properties:
            fills = self.properties["fills"]
            if fills and len(fills) > 0:
                fill = fills[0]
                if fill.get("type") == "SOLID":
                    color = fill.get("color", {})
                    ui_props["color"] = f"rgb({int(color.get('r', 0)*255)}, {int(color.get('g', 0)*255)}, {int(color.get('b', 0)*255)})"
        
        # Extract font properties
        if "style" in self.properties:
            style = self.properties["style"]
            if "fontSize" in style:
                ui_props["font_size"] = style["fontSize"]
        
        return ui_props


class FigmaUITool:
    """
    HTTP tool for integrating with Figma API and AI-generated UI designs.
    
    Supports:
    - Fetching designs from Figma API
    - Processing AI-generated design JSON
    - Converting designs to UI specification format
    - Caching and optimization
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        self.config = config or {}
        self.figma_token = self.config.get("figma_token")
        self.base_url = "https://api.figma.com/v1"
        self.session: Optional[aiohttp.ClientSession] = None
        self.logger = logging.getLogger("figma_ui_tool")
        
        # AI design service configuration
        self.ai_design_services = self.config.get("ai_design_services", {})
        
        # Cache for design data
        self.design_cache: Dict[str, Dict[str, Any]] = {}
    
    async def __aenter__(self):
        """Async context manager entry"""
        self.session = aiohttp.ClientSession()
        return self
    
    async def __aexit__(self, exc_type, exc_val, exc_tb):
        """Async context manager exit"""
        if self.session:
            await self.session.close()
    
    async def fetch_figma_design(self, file_id: str, node_id: Optional[str] = None) -> Dict[str, Any]:
        """
        Fetch design from Figma API.
        
        Args:
            file_id: Figma file ID
            node_id: Optional specific node ID to fetch
            
        Returns:
            UI specification in agent format
        """
        
        if not self.figma_token:
            raise ValueError("Figma token not configured")
        
        if not self.session:
            raise RuntimeError("Tool not initialized. Use async context manager.")
        
        try:
            # Check cache first
            cache_key = f"figma_{file_id}_{node_id or 'root'}"
            if cache_key in self.design_cache:
                self.logger.info(f"Using cached design for {cache_key}")
                return self.design_cache[cache_key]
            
            # Fetch file data from Figma
            headers = {"X-Figma-Token": self.figma_token}
            url = f"{self.base_url}/files/{file_id}"
            
            if node_id:
                url += f"/nodes?ids={node_id}"
            
            self.logger.info(f"Fetching Figma design from: {url}")
            
            async with self.session.get(url, headers=headers) as response:
                if response.status != 200:
                    raise Exception(f"Figma API error: {response.status} - {await response.text()}")
                
                figma_data = await response.json()
            
            # Convert Figma data to UI specification
            ui_spec = await self._convert_figma_to_ui_spec(figma_data, file_id, node_id)
            
            # Cache the result
            self.design_cache[cache_key] = ui_spec
            
            return ui_spec
            
        except Exception as e:
            self.logger.error(f"Error fetching Figma design: {e}")
            raise
    
    async def process_ai_design(self, design_data: Dict[str, Any], 
                              design_source: str = "ai_generated") -> Dict[str, Any]:
        """
        Process AI-generated design data.
        
        Args:
            design_data: AI-generated design specification
            design_source: Source identifier for the AI service
            
        Returns:
            UI specification in agent format
        """
        
        try:
            self.logger.info(f"Processing AI design from {design_source}")
            
            # Check cache
            cache_key = f"ai_{design_source}_{hash(json.dumps(design_data, sort_keys=True))}"
            if cache_key in self.design_cache:
                return self.design_cache[cache_key]
            
            # Convert AI design to UI specification
            ui_spec = await self._convert_ai_design_to_ui_spec(design_data, design_source)
            
            # Cache the result
            self.design_cache[cache_key] = ui_spec
            
            return ui_spec
            
        except Exception as e:
            self.logger.error(f"Error processing AI design: {e}")
            raise
    
    async def fetch_from_url(self, design_url: str, format_type: str = "auto") -> Dict[str, Any]:
        """
        Fetch design from a URL.
        
        Args:
            design_url: URL to fetch design from
            format_type: Expected format (figma, json, auto)
            
        Returns:
            UI specification in agent format
        """
        
        if not self.session:
            raise RuntimeError("Tool not initialized. Use async context manager.")
        
        try:
            self.logger.info(f"Fetching design from URL: {design_url}")
            
            # Detect format if auto
            if format_type == "auto":
                if "figma.com" in design_url:
                    format_type = "figma"
                else:
                    format_type = "json"
            
            if format_type == "figma":
                # Extract file ID from Figma URL
                file_id = self._extract_figma_file_id(design_url)
                node_id = self._extract_figma_node_id(design_url)
                return await self.fetch_figma_design(file_id, node_id)
            
            elif format_type == "json":
                # Fetch JSON data directly
                async with self.session.get(design_url) as response:
                    if response.status != 200:
                        raise Exception(f"HTTP error: {response.status}")
                    
                    design_data = await response.json()
                    return await self.process_ai_design(design_data, "url_json")
            
            else:
                raise ValueError(f"Unsupported format type: {format_type}")
                
        except Exception as e:
            self.logger.error(f"Error fetching design from URL: {e}")
            raise
    
    async def _convert_figma_to_ui_spec(self, figma_data: Dict[str, Any], 
                                      file_id: str, node_id: Optional[str]) -> Dict[str, Any]:
        """Convert Figma API response to UI specification"""
        
        # Extract document or specific nodes
        if node_id:
            nodes = figma_data.get("nodes", {})
            if node_id not in nodes:
                raise ValueError(f"Node {node_id} not found in Figma response")
            root_node = nodes[node_id]["document"]
        else:
            root_node = figma_data.get("document")
            if not root_node:
                raise ValueError("No document found in Figma response")
        
        # Convert Figma nodes to UI components
        figma_nodes = self._parse_figma_nodes(root_node)
        ui_components = [node.to_ui_component() for node in figma_nodes]
        
        # Generate widget name
        widget_name = self._generate_widget_name_from_figma(figma_data, file_id)
        
        # Build UI specification
        ui_spec = {
            "widget_name": widget_name,
            "description": f"Imported from Figma file {file_id}",
            "components": ui_components,
            "layout_type": self._determine_layout_type_from_figma(figma_nodes),
            "interactions": self._extract_figma_interactions(figma_nodes)
        }
        
        return ui_spec
    
    async def _convert_ai_design_to_ui_spec(self, design_data: Dict[str, Any], 
                                          source: str) -> Dict[str, Any]:
        """Convert AI-generated design to UI specification"""
        
        # Handle different AI design formats
        if "components" in design_data:
            # Already in UI component format
            return design_data
        
        elif "elements" in design_data:
            # Convert elements to components
            components = []
            for i, element in enumerate(design_data["elements"]):
                component = {
                    "id": f"ai_component_{i}",
                    "type": element.get("type", "Button"),
                    "label": element.get("text", element.get("label", f"Component {i}")),
                    "description": element.get("description", ""),
                    "position": element.get("position", {"x": 0, "y": 0}),
                    "size": element.get("size", {"width": 100, "height": 50}),
                    "properties": element.get("properties", {}),
                    "children": []
                }
                components.append(component)
            
            return {
                "widget_name": design_data.get("name", "AIGeneratedWidget"),
                "description": f"AI-generated design from {source}",
                "components": components,
                "layout_type": design_data.get("layout", "canvas"),
                "interactions": design_data.get("interactions", [])
            }
        
        else:
            raise ValueError(f"Unsupported AI design format from {source}")
    
    def _parse_figma_nodes(self, node: Dict[str, Any], parent: Optional[FigmaNode] = None) -> List[FigmaNode]:
        """Recursively parse Figma nodes"""
        
        nodes = []
        
        # Create current node
        figma_node = FigmaNode(
            id=node.get("id", ""),
            name=node.get("name", ""),
            type=node.get("type", ""),
            position=self._extract_figma_position(node),
            size=self._extract_figma_size(node),
            properties=node,
            children=[]
        )
        
        # Parse children
        for child in node.get("children", []):
            child_nodes = self._parse_figma_nodes(child, figma_node)
            figma_node.children.extend(child_nodes)
        
        nodes.append(figma_node)
        return nodes
    
    def _extract_figma_position(self, node: Dict[str, Any]) -> Dict[str, float]:
        """Extract position from Figma node"""
        absolute_bounding_box = node.get("absoluteBoundingBox", {})
        return {
            "x": absolute_bounding_box.get("x", 0.0),
            "y": absolute_bounding_box.get("y", 0.0)
        }
    
    def _extract_figma_size(self, node: Dict[str, Any]) -> Dict[str, float]:
        """Extract size from Figma node"""
        absolute_bounding_box = node.get("absoluteBoundingBox", {})
        return {
            "width": absolute_bounding_box.get("width", 100.0),
            "height": absolute_bounding_box.get("height", 50.0)
        }
    
    def _generate_widget_name_from_figma(self, figma_data: Dict[str, Any], file_id: str) -> str:
        """Generate widget name from Figma data"""
        
        # Try to get name from document
        document = figma_data.get("document", {})
        name = document.get("name", "")
        
        if not name:
            # Use file name if available
            name = figma_data.get("name", f"FigmaWidget_{file_id[:8]}")
        
        # Clean and format name
        clean_name = "".join(c for c in name if c.isalnum() or c in " _")
        pascal_name = "".join(word.capitalize() for word in clean_name.split())
        
        return f"WBP_{pascal_name}"
    
    def _determine_layout_type_from_figma(self, nodes: List[FigmaNode]) -> str:
        """Determine layout type from Figma nodes"""
        
        # Simple heuristic based on node arrangement
        if len(nodes) <= 1:
            return "canvas"
        
        # Check if nodes are arranged vertically or horizontally
        positions = [node.position for node in nodes]
        
        # Calculate variance in x and y positions
        x_positions = [pos["x"] for pos in positions]
        y_positions = [pos["y"] for pos in positions]
        
        x_variance = max(x_positions) - min(x_positions) if x_positions else 0
        y_variance = max(y_positions) - min(y_positions) if y_positions else 0
        
        if x_variance > y_variance * 2:
            return "horizontal"
        elif y_variance > x_variance * 2:
            return "vertical"
        else:
            return "canvas"
    
    def _extract_figma_interactions(self, nodes: List[FigmaNode]) -> List[Dict[str, Any]]:
        """Extract interactions from Figma nodes"""
        
        interactions = []
        
        for node in nodes:
            # Look for button-like nodes
            if ("button" in node.name.lower() or 
                node.type in ["COMPONENT", "INSTANCE"] or
                "click" in node.name.lower()):
                
                interactions.append({
                    "component_id": node.id,
                    "event_type": "OnClicked",
                    "action": f"Handle{node.name.replace(' ', '')}Click"
                })
        
        return interactions
    
    def _extract_figma_file_id(self, figma_url: str) -> str:
        """Extract file ID from Figma URL"""
        
        # Example: https://www.figma.com/file/ABC123/Design-Name
        import re
        match = re.search(r'/file/([a-zA-Z0-9]+)', figma_url)
        if match:
            return match.group(1)
        
        raise ValueError(f"Could not extract file ID from Figma URL: {figma_url}")
    
    def _extract_figma_node_id(self, figma_url: str) -> Optional[str]:
        """Extract node ID from Figma URL if present"""
        
        # Example: https://www.figma.com/file/ABC123/Design-Name?node-id=1%3A2
        import re
        match = re.search(r'node-id=([^&]+)', figma_url)
        if match:
            # URL decode the node ID
            from urllib.parse import unquote
            return unquote(match.group(1))
        
        return None
    
    def clear_cache(self):
        """Clear the design cache"""
        self.design_cache.clear()
        self.logger.info("Design cache cleared")
    
    def get_cache_stats(self) -> Dict[str, Any]:
        """Get cache statistics"""
        return {
            "cached_designs": len(self.design_cache),
            "cache_keys": list(self.design_cache.keys())
        }
