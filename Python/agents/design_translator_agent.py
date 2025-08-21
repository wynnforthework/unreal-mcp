"""
Design Translation Agent

Converts UI JSON schema to UE UMG-compatible widget tree JSON.
This agent validates naming conventions, checks C++ BindWidget compatibility,
and ensures proper UMG component hierarchy.
"""

import asyncio
import json
import re
import time
from typing import Dict, Any, List, Optional, Tuple, Set
from dataclasses import dataclass

from base_agent import BaseAgent, AgentResult, AgentStatus
from schemas import (
    DESIGN_TRANSLATION_INPUT_SCHEMA,
    DESIGN_TRANSLATION_OUTPUT_SCHEMA,
    UIComponentType,
    BindingType
)


@dataclass
class UMGComponent:
    """Represents a UMG component in the widget tree"""
    name: str
    type: str
    position: Dict[str, float]
    size: Dict[str, float]
    properties: Dict[str, Any]
    children: List['UMGComponent']
    parent: Optional['UMGComponent'] = None
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "type": self.type,
            "position": self.position,
            "size": self.size,
            "properties": self.properties,
            "children": [child.to_dict() for child in self.children]
        }


class DesignTranslatorAgent(BaseAgent):
    """
    Agent that translates UI specifications to UMG-compatible widget definitions.
    
    This agent:
    - Converts generic UI components to specific UMG widget types
    - Validates and corrects naming conventions for C++ binding
    - Ensures proper widget hierarchy and layout constraints
    - Generates binding information for C++ integration
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        super().__init__("design_translator", config)
        
        # UMG component type mappings
        self.umg_type_mapping = {
            "Button": UIComponentType.BUTTON.value,
            "TextBlock": UIComponentType.TEXT_BLOCK.value,
            "EditableText": UIComponentType.EDITABLE_TEXT.value,
            "EditableTextBox": UIComponentType.EDITABLE_TEXT_BOX.value,
            "Image": UIComponentType.IMAGE.value,
            "ProgressBar": UIComponentType.PROGRESS_BAR.value,
            "Slider": UIComponentType.SLIDER.value,
            "CheckBox": UIComponentType.CHECK_BOX.value,
            "ComboBoxString": UIComponentType.COMBO_BOX_STRING.value,
            "ListView": UIComponentType.LIST_VIEW.value,
            "Panel": UIComponentType.CANVAS_PANEL.value,
            "Container": UIComponentType.CANVAS_PANEL.value,
            "Border": UIComponentType.BORDER.value,
            "ScrollBox": UIComponentType.SCROLL_BOX.value
        }
        
        # Valid UMG container types
        self.container_types = {
            UIComponentType.CANVAS_PANEL.value,
            UIComponentType.VERTICAL_BOX.value,
            UIComponentType.HORIZONTAL_BOX.value,
            UIComponentType.GRID_PANEL.value,
            UIComponentType.UNIFORM_GRID_PANEL.value,
            UIComponentType.SCROLL_BOX.value,
            UIComponentType.BORDER.value,
            UIComponentType.WIDGET_SWITCHER.value
        }
        
        # C++ naming conventions
        self.cpp_naming_rules = {
            "prefix_mapping": {
                UIComponentType.BUTTON.value: "Btn_",
                UIComponentType.TEXT_BLOCK.value: "Txt_",
                UIComponentType.EDITABLE_TEXT.value: "Edit_",
                UIComponentType.EDITABLE_TEXT_BOX.value: "EditBox_",
                UIComponentType.IMAGE.value: "Img_",
                UIComponentType.PROGRESS_BAR.value: "Progress_",
                UIComponentType.SLIDER.value: "Slider_",
                UIComponentType.CHECK_BOX.value: "Check_",
                UIComponentType.COMBO_BOX_STRING.value: "Combo_",
                UIComponentType.LIST_VIEW.value: "List_",
                UIComponentType.CANVAS_PANEL.value: "Panel_",
                UIComponentType.BORDER.value: "Border_",
                UIComponentType.SCROLL_BOX.value: "Scroll_"
            }
        }
    
    async def execute(self, input_data: Dict[str, Any]) -> AgentResult:
        """Execute the design translation process"""
        start_time = time.time()
        
        try:
            # Validate input
            if not self.validate_input(input_data):
                return self.create_error_result(
                    "INVALID_INPUT",
                    "Input data does not match required schema"
                )
            
            ui_spec = input_data["ui_specification"]
            
            self.logger.info(f"Translating UI specification for widget: {ui_spec['widget_name']}")
            
            # Translate UI specification to UMG format
            umg_definition = await self._translate_to_umg(ui_spec)
            
            # Validate the UMG definition
            validation_results = self._validate_umg_definition(umg_definition)
            
            # Generate C++ bindings
            cpp_bindings = self._generate_cpp_bindings(umg_definition)
            
            # Generate event bindings
            event_bindings = self._generate_event_bindings(ui_spec, umg_definition)
            
            # Create result
            result_data = {
                "umg_widget_definition": {
                    "widget_name": umg_definition["widget_name"],
                    "parent_class": umg_definition.get("parent_class", "UserWidget"),
                    "path": umg_definition.get("path", "/Game/Widgets"),
                    "root_component": umg_definition["root_component"],
                    "cpp_bindings": cpp_bindings,
                    "event_bindings": event_bindings
                },
                "validation_results": validation_results
            }
            
            execution_time = time.time() - start_time
            return self.create_success_result(result_data, execution_time=execution_time)
            
        except Exception as e:
            execution_time = time.time() - start_time
            self.logger.error(f"Error translating design: {e}")
            return self.create_error_result(
                "TRANSLATION_ERROR",
                f"Failed to translate design: {str(e)}",
                execution_time=execution_time
            )
    
    def validate_input(self, input_data: Dict[str, Any]) -> bool:
        """Validate input data against schema"""
        if "ui_specification" not in input_data:
            return False
        
        ui_spec = input_data["ui_specification"]
        required_fields = ["widget_name", "components"]
        
        for field in required_fields:
            if field not in ui_spec:
                return False
        
        if not isinstance(ui_spec["components"], list):
            return False
        
        return True
    
    async def _translate_to_umg(self, ui_spec: Dict[str, Any]) -> Dict[str, Any]:
        """Translate UI specification to UMG widget definition"""
        
        widget_name = ui_spec["widget_name"]
        components = ui_spec["components"]
        layout_type = ui_spec.get("layout_type", "canvas")
        
        # Build component tree
        component_tree = self._build_component_tree(components)
        
        # Convert to UMG components
        umg_root = self._convert_to_umg_components(component_tree, layout_type)
        
        # Apply UMG-specific optimizations
        self._optimize_umg_hierarchy(umg_root)
        
        return {
            "widget_name": widget_name,
            "parent_class": "UserWidget",
            "path": "/Game/Widgets",
            "root_component": umg_root.to_dict()
        }
    
    def _build_component_tree(self, components: List[Dict[str, Any]]) -> Dict[str, Any]:
        """Build a hierarchical component tree from flat component list"""
        
        # Create component lookup
        component_map = {comp["id"]: comp for comp in components}
        
        # Find root component
        root_component = None
        for comp in components:
            if not comp.get("parent_id"):
                root_component = comp
                break
        
        if not root_component:
            # Create default root if none exists
            root_component = {
                "id": "root",
                "type": "CanvasPanel",
                "label": "Root Panel",
                "children": [comp["id"] for comp in components]
            }
            component_map["root"] = root_component
        
        # Build tree structure
        def build_tree(comp_id: str) -> Dict[str, Any]:
            comp = component_map[comp_id].copy()
            comp["children_data"] = []
            
            for child_id in comp.get("children", []):
                if child_id in component_map:
                    comp["children_data"].append(build_tree(child_id))
            
            return comp
        
        return build_tree(root_component["id"])
    
    def _convert_to_umg_components(self, component_tree: Dict[str, Any], 
                                 layout_type: str) -> UMGComponent:
        """Convert component tree to UMG components"""
        
        def convert_component(comp_data: Dict[str, Any], parent: Optional[UMGComponent] = None) -> UMGComponent:
            # Map component type to UMG type
            umg_type = self._map_to_umg_type(comp_data["type"], layout_type, parent)
            
            # Generate proper UMG name
            umg_name = self._generate_umg_name(comp_data["label"], umg_type)
            
            # Convert position and size
            position = self._convert_position(comp_data.get("position", {}))
            size = self._convert_size(comp_data.get("size", {}), umg_type)
            
            # Convert properties
            properties = self._convert_properties(comp_data.get("properties", {}), umg_type)
            
            # Create UMG component
            umg_comp = UMGComponent(
                name=umg_name,
                type=umg_type,
                position=position,
                size=size,
                properties=properties,
                children=[],
                parent=parent
            )
            
            # Convert children
            for child_data in comp_data.get("children_data", []):
                child_comp = convert_component(child_data, umg_comp)
                umg_comp.children.append(child_comp)
            
            return umg_comp
        
        return convert_component(component_tree)
    
    def _map_to_umg_type(self, component_type: str, layout_type: str, 
                        parent: Optional[UMGComponent] = None) -> str:
        """Map generic component type to specific UMG type"""
        
        # Direct mapping if available
        if component_type in self.umg_type_mapping:
            return self.umg_type_mapping[component_type]
        
        # Handle container types based on layout
        if component_type in ["Panel", "Container"]:
            if layout_type == "vertical":
                return UIComponentType.VERTICAL_BOX.value
            elif layout_type == "horizontal":
                return UIComponentType.HORIZONTAL_BOX.value
            elif layout_type == "grid":
                return UIComponentType.GRID_PANEL.value
            else:
                return UIComponentType.CANVAS_PANEL.value
        
        # Default mappings
        type_defaults = {
            "text": UIComponentType.TEXT_BLOCK.value,
            "label": UIComponentType.TEXT_BLOCK.value,
            "input": UIComponentType.EDITABLE_TEXT.value,
            "textbox": UIComponentType.EDITABLE_TEXT_BOX.value,
            "picture": UIComponentType.IMAGE.value,
            "progress": UIComponentType.PROGRESS_BAR.value,
            "dropdown": UIComponentType.COMBO_BOX_STRING.value,
            "list": UIComponentType.LIST_VIEW.value
        }
        
        return type_defaults.get(component_type.lower(), UIComponentType.BUTTON.value)
    
    def _generate_umg_name(self, label: str, umg_type: str) -> str:
        """Generate proper UMG component name following C++ conventions"""
        
        # Clean label for use in name
        clean_label = re.sub(r'[^a-zA-Z0-9]', '', label)
        if not clean_label:
            clean_label = "Component"
        
        # Apply prefix based on type
        prefix = self.cpp_naming_rules["prefix_mapping"].get(umg_type, "")
        
        # Ensure PascalCase
        pascal_label = ''.join(word.capitalize() for word in re.split(r'[^a-zA-Z0-9]', clean_label))
        
        return f"{prefix}{pascal_label}"
    
    def _convert_position(self, position_data: Dict[str, Any]) -> Dict[str, float]:
        """Convert position data to UMG format"""
        default_position = {"x": 0.0, "y": 0.0}
        
        if not position_data:
            return default_position
        
        # Handle anchor-based positioning
        if "anchor" in position_data:
            anchor = position_data["anchor"]
            if anchor == "center":
                return {"x": 0.5, "y": 0.5}
            elif anchor == "top":
                return {"x": 0.5, "y": 0.1}
            elif anchor == "bottom":
                return {"x": 0.5, "y": 0.9}
            elif anchor == "left":
                return {"x": 0.1, "y": 0.5}
            elif anchor == "right":
                return {"x": 0.9, "y": 0.5}
        
        # Handle direct coordinates
        x = position_data.get("x", 0.0)
        y = position_data.get("y", 0.0)
        
        # Normalize to 0-1 range if values are large (assume pixel coordinates)
        if x > 1.0 or y > 1.0:
            x = min(x / 1920.0, 1.0)  # Assume 1920x1080 reference
            y = min(y / 1080.0, 1.0)
        
        return {"x": float(x), "y": float(y)}
    
    def _convert_size(self, size_data: Dict[str, Any], umg_type: str) -> Dict[str, float]:
        """Convert size data to UMG format"""
        
        # Default sizes based on component type
        default_sizes = {
            UIComponentType.BUTTON.value: {"width": 200.0, "height": 50.0},
            UIComponentType.TEXT_BLOCK.value: {"width": 150.0, "height": 30.0},
            UIComponentType.EDITABLE_TEXT.value: {"width": 200.0, "height": 30.0},
            UIComponentType.IMAGE.value: {"width": 100.0, "height": 100.0},
            UIComponentType.CANVAS_PANEL.value: {"width": 800.0, "height": 600.0}
        }
        
        default_size = default_sizes.get(umg_type, {"width": 100.0, "height": 50.0})
        
        if not size_data:
            return default_size
        
        width = size_data.get("width", default_size["width"])
        height = size_data.get("height", default_size["height"])
        
        # Handle special size values
        if width == -1:  # Full width
            width = 800.0
        if height == -1:  # Full height
            height = 600.0
        
        return {"width": float(width), "height": float(height)}
    
    def _convert_properties(self, properties: Dict[str, Any], umg_type: str) -> Dict[str, Any]:
        """Convert generic properties to UMG-specific properties"""
        umg_properties = {}
        
        # Text properties
        if umg_type in [UIComponentType.TEXT_BLOCK.value, UIComponentType.BUTTON.value]:
            if "color" in properties:
                umg_properties["text_color"] = properties["color"]
            if "font_size" in properties:
                umg_properties["font_size"] = properties["font_size"]
        
        # Button properties
        if umg_type == UIComponentType.BUTTON.value:
            if "is_enabled" in properties:
                umg_properties["is_enabled"] = properties["is_enabled"]
        
        # Image properties
        if umg_type == UIComponentType.IMAGE.value:
            if "color" in properties:
                umg_properties["color_and_opacity"] = properties["color"]
        
        return umg_properties
    
    def _optimize_umg_hierarchy(self, root: UMGComponent):
        """Apply UMG-specific optimizations to the component hierarchy"""
        
        # Ensure root is a valid container
        if root.type not in self.container_types:
            # Wrap in a canvas panel
            canvas = UMGComponent(
                name="RootCanvas",
                type=UIComponentType.CANVAS_PANEL.value,
                position={"x": 0.0, "y": 0.0},
                size={"width": 800.0, "height": 600.0},
                properties={},
                children=[root]
            )
            root.parent = canvas
            return canvas
        
        # Remove unnecessary nesting
        self._remove_unnecessary_containers(root)
        
        return root
    
    def _remove_unnecessary_containers(self, component: UMGComponent):
        """Remove unnecessary container components"""
        
        # If a container has only one child and serves no purpose, consider flattening
        if (component.type in self.container_types and 
            len(component.children) == 1 and 
            not component.properties):
            
            child = component.children[0]
            if child.type in self.container_types:
                # Move grandchildren up
                component.children = child.children
                for grandchild in child.children:
                    grandchild.parent = component
        
        # Recursively process children
        for child in component.children:
            self._remove_unnecessary_containers(child)
    
    def _validate_umg_definition(self, umg_definition: Dict[str, Any]) -> Dict[str, Any]:
        """Validate the UMG definition for correctness"""
        
        validation_results = {
            "naming_validation": True,
            "hierarchy_validation": True,
            "binding_validation": True,
            "warnings": [],
            "errors": []
        }
        
        # Validate naming conventions
        self._validate_naming(umg_definition["root_component"], validation_results)
        
        # Validate hierarchy
        self._validate_hierarchy(umg_definition["root_component"], validation_results)
        
        # Update overall validation status
        validation_results["naming_validation"] = len([w for w in validation_results["warnings"] if "naming" in w.lower()]) == 0
        validation_results["hierarchy_validation"] = len([e for e in validation_results["errors"] if "hierarchy" in e.lower()]) == 0
        
        return validation_results
    
    def _validate_naming(self, component: Dict[str, Any], results: Dict[str, Any]):
        """Validate component naming conventions"""
        
        name = component["name"]
        comp_type = component["type"]
        
        # Check for valid C++ identifier
        if not re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', name):
            results["errors"].append(f"Invalid C++ identifier: {name}")
        
        # Check for proper prefix
        expected_prefix = self.cpp_naming_rules["prefix_mapping"].get(comp_type, "")
        if expected_prefix and not name.startswith(expected_prefix):
            results["warnings"].append(f"Component {name} should start with {expected_prefix}")
        
        # Recursively validate children
        for child in component.get("children", []):
            self._validate_naming(child, results)
    
    def _validate_hierarchy(self, component: Dict[str, Any], results: Dict[str, Any]):
        """Validate component hierarchy"""
        
        comp_type = component["type"]
        
        # Check if component can have children
        if component.get("children") and comp_type not in self.container_types:
            results["errors"].append(f"Non-container component {component['name']} cannot have children")
        
        # Recursively validate children
        for child in component.get("children", []):
            self._validate_hierarchy(child, results)
    
    def _generate_cpp_bindings(self, umg_definition: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Generate C++ binding information"""
        bindings = []
        
        def collect_bindings(component: Dict[str, Any]):
            # Only bind interactive components
            bindable_types = {
                UIComponentType.BUTTON.value,
                UIComponentType.TEXT_BLOCK.value,
                UIComponentType.EDITABLE_TEXT.value,
                UIComponentType.EDITABLE_TEXT_BOX.value,
                UIComponentType.IMAGE.value,
                UIComponentType.PROGRESS_BAR.value,
                UIComponentType.SLIDER.value,
                UIComponentType.CHECK_BOX.value,
                UIComponentType.COMBO_BOX_STRING.value,
                UIComponentType.LIST_VIEW.value
            }
            
            if component["type"] in bindable_types:
                bindings.append({
                    "widget_name": component["name"],
                    "cpp_variable_name": component["name"],
                    "widget_type": component["type"],
                    "binding_type": BindingType.BIND_WIDGET.value,
                    "is_optional": False
                })
            
            # Process children
            for child in component.get("children", []):
                collect_bindings(child)
        
        collect_bindings(umg_definition["root_component"])
        return bindings
    
    def _generate_event_bindings(self, ui_spec: Dict[str, Any], 
                                umg_definition: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Generate event binding information"""
        event_bindings = []
        
        interactions = ui_spec.get("interactions", [])
        component_map = self._build_component_map(umg_definition["root_component"])
        
        for interaction in interactions:
            component_id = interaction["component_id"]
            event_type = interaction["event_type"]
            action = interaction["action"]
            
            # Find corresponding UMG component
            umg_component = self._find_component_by_original_id(component_map, component_id)
            if umg_component:
                function_name = f"On{umg_component['name']}{event_type.replace('On', '')}"
                event_bindings.append({
                    "widget_name": umg_component["name"],
                    "event_type": event_type,
                    "function_name": function_name
                })
        
        return event_bindings
    
    def _build_component_map(self, root_component: Dict[str, Any]) -> Dict[str, Dict[str, Any]]:
        """Build a map of all components in the tree"""
        component_map = {}
        
        def map_components(component: Dict[str, Any]):
            component_map[component["name"]] = component
            for child in component.get("children", []):
                map_components(child)
        
        map_components(root_component)
        return component_map
    
    def _find_component_by_original_id(self, component_map: Dict[str, Dict[str, Any]], 
                                     original_id: str) -> Optional[Dict[str, Any]]:
        """Find UMG component by original component ID"""
        # This is a simplified lookup - in practice, you'd maintain a mapping
        # between original IDs and UMG component names
        for component in component_map.values():
            if original_id in component.get("name", "").lower():
                return component
        return None
    
    def _get_input_schema(self) -> Dict[str, Any]:
        """Get input schema for this agent"""
        return DESIGN_TRANSLATION_INPUT_SCHEMA
    
    def _get_output_schema(self) -> Dict[str, Any]:
        """Get output schema for this agent"""
        return DESIGN_TRANSLATION_OUTPUT_SCHEMA
