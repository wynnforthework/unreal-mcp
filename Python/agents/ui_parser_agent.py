"""
UI Requirements Parser Agent

Converts natural language UI requirements into structured JSON schema.
This agent uses LLM capabilities with JSON schema constraints to parse
user requests and extract UI component specifications.
"""

import asyncio
import json
import re
import time
from typing import Dict, Any, List, Optional, Tuple
from dataclasses import dataclass

from .base_agent import BaseAgent, AgentResult, AgentStatus
from .schemas import (
    UI_REQUIREMENTS_INPUT_SCHEMA,
    UI_REQUIREMENTS_OUTPUT_SCHEMA,
    UIComponentType
)


@dataclass
class UIComponent:
    """Represents a UI component extracted from natural language"""
    id: str
    type: str
    label: str
    description: str
    position: Optional[Dict[str, Any]] = None
    size: Optional[Dict[str, Any]] = None
    properties: Optional[Dict[str, Any]] = None
    parent_id: Optional[str] = None
    children: List[str] = None
    
    def __post_init__(self):
        if self.children is None:
            self.children = []
        if self.properties is None:
            self.properties = {}


class UIParserAgent(BaseAgent):
    """
    Agent that parses natural language UI requirements into structured JSON.
    
    This agent analyzes user input to identify:
    - UI components and their types
    - Layout relationships and hierarchy
    - Component properties and styling
    - Interaction requirements
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        super().__init__("ui_parser", config)
        
        # Component type mappings for natural language
        self.component_keywords = {
            "button": UIComponentType.BUTTON.value,
            "text": UIComponentType.TEXT_BLOCK.value,
            "label": UIComponentType.TEXT_BLOCK.value,
            "input": UIComponentType.EDITABLE_TEXT.value,
            "textbox": UIComponentType.EDITABLE_TEXT_BOX.value,
            "image": UIComponentType.IMAGE.value,
            "picture": UIComponentType.IMAGE.value,
            "slider": UIComponentType.SLIDER.value,
            "checkbox": UIComponentType.CHECK_BOX.value,
            "dropdown": UIComponentType.COMBO_BOX_STRING.value,
            "combobox": UIComponentType.COMBO_BOX_STRING.value,
            "list": UIComponentType.LIST_VIEW.value,
            "progress": UIComponentType.PROGRESS_BAR.value,
            "progressbar": UIComponentType.PROGRESS_BAR.value,
            "panel": UIComponentType.CANVAS_PANEL.value,
            "container": UIComponentType.CANVAS_PANEL.value,
            "border": UIComponentType.BORDER.value,
            "scroll": UIComponentType.SCROLL_BOX.value,
            "scrollbox": UIComponentType.SCROLL_BOX.value
        }
        
        # Position keywords
        self.position_keywords = {
            "top": {"anchor": "top", "y": 0.1},
            "bottom": {"anchor": "bottom", "y": 0.9},
            "left": {"anchor": "left", "x": 0.1},
            "right": {"anchor": "right", "x": 0.9},
            "center": {"anchor": "center", "x": 0.5, "y": 0.5},
            "middle": {"anchor": "center", "x": 0.5, "y": 0.5},
            "upper": {"anchor": "top", "y": 0.2},
            "lower": {"anchor": "bottom", "y": 0.8},
            "corner": {"anchor": "corner"}
        }
        
        # Size keywords
        self.size_keywords = {
            "small": {"width": 100, "height": 50},
            "medium": {"width": 200, "height": 100},
            "large": {"width": 300, "height": 150},
            "big": {"width": 300, "height": 150},
            "wide": {"width": 400, "height": 100},
            "tall": {"width": 150, "height": 300},
            "full": {"width": -1, "height": -1}  # Full size
        }
    
    async def execute(self, input_data: Dict[str, Any]) -> AgentResult:
        """Execute the UI parsing process"""
        start_time = time.time()
        
        try:
            # Validate input
            if not self.validate_input(input_data):
                return self.create_error_result(
                    "INVALID_INPUT",
                    "Input data does not match required schema"
                )
            
            natural_language = input_data["natural_language_request"]
            context = input_data.get("context", {})
            
            self.logger.info(f"Parsing UI requirements: {natural_language[:100]}...")
            
            # Parse the natural language request
            ui_spec = await self._parse_ui_requirements(natural_language, context)
            
            # Calculate confidence score
            confidence = self._calculate_confidence(natural_language, ui_spec)
            
            # Create result
            result_data = {
                "ui_specification": ui_spec,
                "confidence": confidence,
                "parsing_metadata": {
                    "input_length": len(natural_language),
                    "components_found": len(ui_spec.get("components", [])),
                    "parsing_method": "rule_based_nlp"
                }
            }
            
            execution_time = time.time() - start_time
            return self.create_success_result(result_data, execution_time=execution_time)
            
        except Exception as e:
            execution_time = time.time() - start_time
            self.logger.error(f"Error parsing UI requirements: {e}")
            return self.create_error_result(
                "PARSING_ERROR",
                f"Failed to parse UI requirements: {str(e)}",
                execution_time=execution_time
            )
    
    def validate_input(self, input_data: Dict[str, Any]) -> bool:
        """Validate input data against schema"""
        required_fields = ["natural_language_request"]
        
        for field in required_fields:
            if field not in input_data:
                return False
        
        if not isinstance(input_data["natural_language_request"], str):
            return False
        
        if len(input_data["natural_language_request"].strip()) == 0:
            return False
        
        return True
    
    async def _parse_ui_requirements(self, text: str, context: Dict[str, Any]) -> Dict[str, Any]:
        """Parse natural language text into UI specification"""
        
        # Clean and normalize text
        text = text.lower().strip()
        
        # Extract widget name
        widget_name = self._extract_widget_name(text)
        
        # Extract components
        components = self._extract_components(text)
        
        # Determine layout type
        layout_type = self._determine_layout_type(text, components)
        
        # Extract interactions
        interactions = self._extract_interactions(text, components)
        
        # Build component hierarchy
        self._build_component_hierarchy(components, layout_type)
        
        return {
            "widget_name": widget_name,
            "description": text,
            "components": [comp.__dict__ for comp in components],
            "layout_type": layout_type,
            "interactions": interactions
        }
    
    def _extract_widget_name(self, text: str) -> str:
        """Extract widget name from text"""
        # Look for explicit widget names
        name_patterns = [
            r"(?:create|make|build)\s+(?:a|an)?\s*(\w+(?:\s+\w+)*?)(?:\s+(?:widget|ui|interface|menu|screen|page))",
            r"(?:widget|ui|interface|menu|screen|page)\s+(?:called|named)\s+(\w+(?:\s+\w+)*)",
            r"(\w+(?:\s+\w+)*?)\s+(?:widget|ui|interface|menu|screen|page)"
        ]
        
        for pattern in name_patterns:
            match = re.search(pattern, text)
            if match:
                name = match.group(1).strip()
                # Convert to PascalCase
                return "WBP_" + "".join(word.capitalize() for word in name.split())
        
        # Default names based on content
        if "menu" in text:
            return "WBP_MainMenu"
        elif "hud" in text or "interface" in text:
            return "WBP_GameHUD"
        elif "settings" in text or "options" in text:
            return "WBP_SettingsMenu"
        else:
            return "WBP_CustomWidget"
    
    def _extract_components(self, text: str) -> List[UIComponent]:
        """Extract UI components from text"""
        components = []
        component_id = 1
        
        # Split text into sentences for better parsing
        sentences = re.split(r'[.!?;,]', text)
        
        for sentence in sentences:
            sentence = sentence.strip()
            if not sentence:
                continue
            
            # Find component types in sentence
            found_components = self._find_components_in_sentence(sentence, component_id)
            components.extend(found_components)
            component_id += len(found_components)
        
        # Ensure we have at least a root container
        if not any(comp.type in ["CanvasPanel", "VerticalBox", "HorizontalBox"] for comp in components):
            root = UIComponent(
                id="root_panel",
                type="CanvasPanel",
                label="Root Panel",
                description="Main container panel"
            )
            components.insert(0, root)
        
        return components
    
    def _find_components_in_sentence(self, sentence: str, start_id: int) -> List[UIComponent]:
        """Find components in a single sentence"""
        components = []
        
        # Look for component keywords
        for keyword, component_type in self.component_keywords.items():
            if keyword in sentence:
                # Extract label/text for the component
                label = self._extract_component_label(sentence, keyword)
                
                # Extract position information
                position = self._extract_position(sentence)
                
                # Extract size information
                size = self._extract_size(sentence)
                
                # Extract properties
                properties = self._extract_properties(sentence, component_type)
                
                component = UIComponent(
                    id=f"component_{start_id + len(components)}",
                    type=component_type,
                    label=label,
                    description=sentence,
                    position=position,
                    size=size,
                    properties=properties
                )
                
                components.append(component)
        
        return components
    
    def _extract_component_label(self, sentence: str, keyword: str) -> str:
        """Extract the label/text for a component"""
        # Look for quoted text
        quote_match = re.search(r'["\']([^"\']+)["\']', sentence)
        if quote_match:
            return quote_match.group(1)
        
        # Look for text after "says", "shows", "displays"
        text_patterns = [
            rf"{keyword}\s+(?:that\s+)?(?:says|shows|displays|with\s+text)\s+[\"']?([^\"',.]+)[\"']?",
            rf"[\"']([^\"']+)[\"']\s+{keyword}",
            rf"{keyword}\s+[\"']([^\"']+)[\"']"
        ]
        
        for pattern in text_patterns:
            match = re.search(pattern, sentence)
            if match:
                return match.group(1).strip()
        
        # Default labels based on component type
        defaults = {
            "Button": "Button",
            "TextBlock": "Text",
            "EditableText": "Input",
            "Image": "Image"
        }
        
        return defaults.get(keyword.capitalize(), keyword.capitalize())
    
    def _extract_position(self, sentence: str) -> Optional[Dict[str, Any]]:
        """Extract position information from sentence"""
        position = {}
        
        for keyword, pos_info in self.position_keywords.items():
            if keyword in sentence:
                position.update(pos_info)
                break
        
        # Look for specific coordinates
        coord_match = re.search(r'at\s+(?:position\s+)?(?:\()?(\d+),?\s*(\d+)(?:\))?', sentence)
        if coord_match:
            position.update({
                "x": int(coord_match.group(1)),
                "y": int(coord_match.group(2))
            })
        
        return position if position else None
    
    def _extract_size(self, sentence: str) -> Optional[Dict[str, Any]]:
        """Extract size information from sentence"""
        size = {}
        
        for keyword, size_info in self.size_keywords.items():
            if keyword in sentence:
                size.update(size_info)
                break
        
        # Look for specific dimensions
        dim_match = re.search(r'(?:size|dimensions?)\s+(?:of\s+)?(?:\()?(\d+)x(\d+)(?:\))?', sentence)
        if dim_match:
            size.update({
                "width": int(dim_match.group(1)),
                "height": int(dim_match.group(2))
            })
        
        return size if size else None
    
    def _extract_properties(self, sentence: str, component_type: str) -> Dict[str, Any]:
        """Extract component-specific properties"""
        properties = {}
        
        # Color extraction
        color_match = re.search(r'(?:color|colored)\s+(\w+)', sentence)
        if color_match:
            properties["color"] = color_match.group(1)
        
        # Font size for text components
        if component_type in ["TextBlock", "EditableText", "EditableTextBox"]:
            font_match = re.search(r'(?:font\s+size|size)\s+(\d+)', sentence)
            if font_match:
                properties["font_size"] = int(font_match.group(1))
        
        # Button specific properties
        if component_type == "Button":
            if "disabled" in sentence or "inactive" in sentence:
                properties["is_enabled"] = False
        
        return properties
    
    def _determine_layout_type(self, text: str, components: List[UIComponent]) -> str:
        """Determine the layout type based on text and components"""
        if "vertical" in text or "column" in text:
            return "vertical"
        elif "horizontal" in text or "row" in text:
            return "horizontal"
        elif "grid" in text:
            return "grid"
        else:
            return "canvas"  # Default to canvas layout
    
    def _extract_interactions(self, text: str, components: List[UIComponent]) -> List[Dict[str, Any]]:
        """Extract interaction requirements"""
        interactions = []
        
        # Look for click interactions
        click_patterns = [
            r'(?:click|press|tap)\s+(?:the\s+)?(\w+(?:\s+\w+)*?)\s+(?:to|and|will)\s+(\w+(?:\s+\w+)*)',
            r'(\w+(?:\s+\w+)*?)\s+(?:button|link)\s+(?:to|will)\s+(\w+(?:\s+\w+)*)'
        ]
        
        for pattern in click_patterns:
            matches = re.finditer(pattern, text)
            for match in matches:
                component_name = match.group(1).strip()
                action = match.group(2).strip()
                
                # Find matching component
                for comp in components:
                    if component_name.lower() in comp.label.lower():
                        interactions.append({
                            "component_id": comp.id,
                            "event_type": "OnClicked",
                            "action": action
                        })
                        break
        
        return interactions
    
    def _build_component_hierarchy(self, components: List[UIComponent], layout_type: str):
        """Build parent-child relationships between components"""
        if not components:
            return
        
        # Find or create root container
        root_component = None
        for comp in components:
            if comp.type in ["CanvasPanel", "VerticalBox", "HorizontalBox", "GridPanel"]:
                root_component = comp
                break
        
        if not root_component:
            # Create default root
            root_component = components[0]
            root_component.type = "CanvasPanel"
        
        # Assign other components as children of root
        for comp in components:
            if comp != root_component and not comp.parent_id:
                comp.parent_id = root_component.id
                root_component.children.append(comp.id)
    
    def _calculate_confidence(self, text: str, ui_spec: Dict[str, Any]) -> float:
        """Calculate confidence score for the parsing result"""
        confidence = 0.5  # Base confidence
        
        # Increase confidence based on found components
        components_count = len(ui_spec.get("components", []))
        if components_count > 0:
            confidence += min(0.3, components_count * 0.1)
        
        # Increase confidence if specific UI terms are found
        ui_terms = ["button", "text", "menu", "interface", "widget", "panel"]
        found_terms = sum(1 for term in ui_terms if term in text.lower())
        confidence += min(0.2, found_terms * 0.05)
        
        # Decrease confidence for very short or vague descriptions
        if len(text.split()) < 5:
            confidence -= 0.2
        
        return max(0.0, min(1.0, confidence))
    
    def _get_input_schema(self) -> Dict[str, Any]:
        """Get input schema for this agent"""
        return UI_REQUIREMENTS_INPUT_SCHEMA
    
    def _get_output_schema(self) -> Dict[str, Any]:
        """Get output schema for this agent"""
        return UI_REQUIREMENTS_OUTPUT_SCHEMA
