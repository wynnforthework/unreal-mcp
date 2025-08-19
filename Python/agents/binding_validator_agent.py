"""
Binding Validation Agent

Validates C++ class header files (scanning UPROPERTY(meta=(BindWidget))) against 
generated blueprint JSON to ensure proper widget binding and report any missing 
or incorrectly named components.
"""

import asyncio
import json
import re
import time
import os
from typing import Dict, Any, List, Optional, Tuple, Set
from dataclasses import dataclass
from pathlib import Path

from .base_agent import BaseAgent, AgentResult, AgentStatus
from .schemas import (
    BINDING_VALIDATION_INPUT_SCHEMA,
    BINDING_VALIDATION_OUTPUT_SCHEMA,
    BindingType
)


@dataclass
class CppBinding:
    """Represents a C++ widget binding found in header file"""
    variable_name: str
    widget_type: str
    binding_type: str
    is_optional: bool
    line_number: int
    raw_declaration: str


@dataclass
class BindingValidationResult:
    """Result of validating a single binding"""
    widget_name: str
    cpp_variable_name: str
    status: str  # found, missing, type_mismatch, name_mismatch
    expected_type: str
    actual_type: Optional[str] = None
    suggestions: List[str] = None
    
    def __post_init__(self):
        if self.suggestions is None:
            self.suggestions = []


class BindingValidatorAgent(BaseAgent):
    """
    Agent that validates C++ widget bindings against UMG blueprints.
    
    This agent:
    - Parses C++ header files for UPROPERTY(meta=(BindWidget)) declarations
    - Retrieves widget blueprint layout information
    - Validates that all bindings match between C++ and blueprint
    - Provides suggestions for fixing mismatches
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        super().__init__("binding_validator", config)
        
        # TCP connection settings for UE communication
        self.tcp_host = config.get("tcp_host", "127.0.0.1") if config else "127.0.0.1"
        self.tcp_port = config.get("tcp_port", 55557) if config else 55557
        
        # Widget type mappings for validation
        self.widget_type_mappings = {
            "UButton": "Button",
            "UTextBlock": "TextBlock", 
            "UEditableText": "EditableText",
            "UEditableTextBox": "EditableTextBox",
            "UImage": "Image",
            "UProgressBar": "ProgressBar",
            "USlider": "Slider",
            "UCheckBox": "CheckBox",
            "UComboBoxString": "ComboBoxString",
            "UListView": "ListView",
            "UCanvasPanel": "CanvasPanel",
            "UVerticalBox": "VerticalBox",
            "UHorizontalBox": "HorizontalBox",
            "UGridPanel": "GridPanel",
            "UBorder": "Border",
            "UScrollBox": "ScrollBox"
        }
        
        # Reverse mapping for suggestions
        self.blueprint_to_cpp_types = {v: k for k, v in self.widget_type_mappings.items()}
    
    async def execute(self, input_data: Dict[str, Any]) -> AgentResult:
        """Execute the binding validation process"""
        start_time = time.time()
        
        try:
            # Validate input
            if not self.validate_input(input_data):
                return self.create_error_result(
                    "INVALID_INPUT",
                    "Input data does not match required schema"
                )
            
            widget_blueprint_path = input_data["widget_blueprint_path"]
            cpp_header_path = input_data.get("cpp_header_path")
            expected_bindings = input_data.get("expected_bindings", [])
            
            self.logger.info(f"Validating bindings for widget: {widget_blueprint_path}")
            
            # Get widget layout from UE
            widget_layout = await self._get_widget_layout(widget_blueprint_path)
            
            # Parse C++ bindings if header path provided
            cpp_bindings = []
            if cpp_header_path:
                cpp_bindings = await self._parse_cpp_bindings(cpp_header_path)
            
            # Perform validation
            validation_report = await self._validate_bindings(
                widget_layout, cpp_bindings, expected_bindings
            )
            
            # Create result
            result_data = {
                "validation_report": validation_report
            }
            
            execution_time = time.time() - start_time
            return self.create_success_result(result_data, execution_time=execution_time)
            
        except Exception as e:
            execution_time = time.time() - start_time
            self.logger.error(f"Error validating bindings: {e}")
            return self.create_error_result(
                "VALIDATION_ERROR",
                f"Failed to validate bindings: {str(e)}",
                execution_time=execution_time
            )
    
    def validate_input(self, input_data: Dict[str, Any]) -> bool:
        """Validate input data against schema"""
        required_fields = ["widget_blueprint_path"]
        
        for field in required_fields:
            if field not in input_data:
                return False
        
        return True
    
    async def _get_widget_layout(self, widget_blueprint_path: str) -> Dict[str, Any]:
        """Get widget layout information from UE"""
        
        try:
            # Extract widget name from path
            widget_name = Path(widget_blueprint_path).stem
            
            # Send command to get widget layout
            command_data = {
                "type": "get_widget_component_layout",
                "params": {"widget_name": widget_name}
            }
            
            json_data = json.dumps(command_data)
            
            # Connect to UE TCP server
            reader, writer = await asyncio.open_connection(self.tcp_host, self.tcp_port)
            
            # Send command
            writer.write(json_data.encode('utf-8'))
            writer.write(b'\n')
            await writer.drain()
            
            # Read response
            response_data = await reader.read(8192)
            response_str = response_data.decode('utf-8').strip()
            
            # Close connection
            writer.close()
            await writer.wait_closed()
            
            # Parse response
            if response_str:
                result = json.loads(response_str)
                if result.get("success", False):
                    return result.get("hierarchy", {})
                else:
                    raise Exception(f"Failed to get widget layout: {result.get('error', 'Unknown error')}")
            else:
                raise Exception("Empty response from UE")
                
        except Exception as e:
            self.logger.error(f"Error getting widget layout: {e}")
            return {}
    
    async def _parse_cpp_bindings(self, cpp_header_path: str) -> List[CppBinding]:
        """Parse C++ header file for widget bindings"""
        
        bindings = []
        
        try:
            if not os.path.exists(cpp_header_path):
                self.logger.warning(f"C++ header file not found: {cpp_header_path}")
                return bindings
            
            with open(cpp_header_path, 'r', encoding='utf-8') as file:
                lines = file.readlines()
            
            # Parse UPROPERTY declarations
            for i, line in enumerate(lines):
                line = line.strip()
                
                # Look for UPROPERTY with BindWidget meta
                if "UPROPERTY" in line and "BindWidget" in line:
                    binding = self._parse_uproperty_line(line, i + 1)
                    if binding:
                        # Look for the variable declaration on the next few lines
                        for j in range(i + 1, min(i + 5, len(lines))):
                            var_line = lines[j].strip()
                            if var_line and not var_line.startswith("//"):
                                var_binding = self._parse_variable_declaration(var_line, binding, j + 1)
                                if var_binding:
                                    bindings.append(var_binding)
                                    break
            
            self.logger.info(f"Found {len(bindings)} C++ widget bindings")
            return bindings
            
        except Exception as e:
            self.logger.error(f"Error parsing C++ header file: {e}")
            return []
    
    def _parse_uproperty_line(self, line: str, line_number: int) -> Optional[Dict[str, Any]]:
        """Parse UPROPERTY line to extract binding information"""
        
        # Extract meta information
        meta_match = re.search(r'meta\s*=\s*\((.*?)\)', line)
        if not meta_match:
            return None
        
        meta_content = meta_match.group(1)
        
        # Check for BindWidget
        binding_type = BindingType.BIND_WIDGET.value
        is_optional = False
        
        if "BindWidgetOptional" in meta_content:
            binding_type = BindingType.BIND_WIDGET_OPTIONAL.value
            is_optional = True
        elif "BindWidgetAnim" in meta_content:
            binding_type = BindingType.BIND_WIDGET_ANIM.value
        elif "BindWidget" in meta_content:
            binding_type = BindingType.BIND_WIDGET.value
        else:
            return None
        
        return {
            "binding_type": binding_type,
            "is_optional": is_optional,
            "line_number": line_number,
            "raw_uproperty": line
        }
    
    def _parse_variable_declaration(self, line: str, uproperty_info: Dict[str, Any], 
                                  line_number: int) -> Optional[CppBinding]:
        """Parse variable declaration line"""
        
        # Match C++ variable declaration pattern
        # Examples: UButton* StartButton; or class UTextBlock* PlayerNameText;
        var_pattern = r'(?:class\s+)?([UF]\w+)\s*\*?\s*(\w+)\s*;'
        match = re.search(var_pattern, line)
        
        if not match:
            return None
        
        widget_type = match.group(1)
        variable_name = match.group(2)
        
        return CppBinding(
            variable_name=variable_name,
            widget_type=widget_type,
            binding_type=uproperty_info["binding_type"],
            is_optional=uproperty_info["is_optional"],
            line_number=line_number,
            raw_declaration=line
        )
    
    async def _validate_bindings(self, widget_layout: Dict[str, Any], 
                                cpp_bindings: List[CppBinding],
                                expected_bindings: List[Dict[str, Any]]) -> Dict[str, Any]:
        """Validate bindings between C++ and blueprint"""
        
        # Collect all widgets from blueprint
        blueprint_widgets = self._collect_blueprint_widgets(widget_layout)
        
        # Create validation results
        binding_results = []
        missing_widgets = []
        extra_widgets = []
        recommendations = []
        
        # Validate C++ bindings against blueprint
        for cpp_binding in cpp_bindings:
            result = self._validate_single_binding(cpp_binding, blueprint_widgets)
            binding_results.append(result.__dict__)
        
        # Validate expected bindings if provided
        for expected in expected_bindings:
            widget_name = expected["widget_name"]
            cpp_var_name = expected["cpp_variable_name"]
            expected_type = expected["widget_type"]
            
            # Check if widget exists in blueprint
            blueprint_widget = blueprint_widgets.get(widget_name)
            if not blueprint_widget:
                missing_widgets.append(widget_name)
                continue
            
            # Check type compatibility
            blueprint_type = blueprint_widget["type"]
            if not self._are_types_compatible(expected_type, blueprint_type):
                result = BindingValidationResult(
                    widget_name=widget_name,
                    cpp_variable_name=cpp_var_name,
                    status="type_mismatch",
                    expected_type=expected_type,
                    actual_type=blueprint_type,
                    suggestions=[f"Change C++ type to {self.blueprint_to_cpp_types.get(blueprint_type, blueprint_type)}"]
                )
                binding_results.append(result.__dict__)
        
        # Find extra widgets in blueprint that aren't bound
        bound_widget_names = {binding.variable_name for binding in cpp_bindings}
        expected_widget_names = {binding["widget_name"] for binding in expected_bindings}
        all_bound_names = bound_widget_names.union(expected_widget_names)
        
        for widget_name in blueprint_widgets:
            if widget_name not in all_bound_names:
                widget_type = blueprint_widgets[widget_name]["type"]
                if widget_type in self.blueprint_to_cpp_types:
                    extra_widgets.append(widget_name)
        
        # Generate recommendations
        recommendations = self._generate_recommendations(
            binding_results, missing_widgets, extra_widgets, blueprint_widgets
        )
        
        # Determine overall status
        overall_status = self._determine_overall_status(binding_results, missing_widgets)
        
        return {
            "overall_status": overall_status,
            "binding_results": binding_results,
            "missing_widgets": missing_widgets,
            "extra_widgets": extra_widgets,
            "recommendations": recommendations
        }
    
    def _collect_blueprint_widgets(self, widget_layout: Dict[str, Any]) -> Dict[str, Dict[str, Any]]:
        """Collect all widgets from blueprint layout recursively"""
        
        widgets = {}
        
        def collect_recursive(component: Dict[str, Any]):
            if "name" in component and "type" in component:
                widgets[component["name"]] = {
                    "type": component["type"],
                    "properties": component.get("properties", {})
                }
            
            # Process children
            for child in component.get("children", []):
                collect_recursive(child)
        
        if widget_layout:
            collect_recursive(widget_layout)
        
        return widgets
    
    def _validate_single_binding(self, cpp_binding: CppBinding, 
                                blueprint_widgets: Dict[str, Dict[str, Any]]) -> BindingValidationResult:
        """Validate a single C++ binding against blueprint widgets"""
        
        variable_name = cpp_binding.variable_name
        cpp_type = cpp_binding.widget_type
        
        # Look for widget with matching name
        if variable_name in blueprint_widgets:
            blueprint_widget = blueprint_widgets[variable_name]
            blueprint_type = blueprint_widget["type"]
            
            # Check type compatibility
            if self._are_types_compatible(cpp_type, blueprint_type):
                return BindingValidationResult(
                    widget_name=variable_name,
                    cpp_variable_name=variable_name,
                    status="found",
                    expected_type=cpp_type,
                    actual_type=blueprint_type
                )
            else:
                return BindingValidationResult(
                    widget_name=variable_name,
                    cpp_variable_name=variable_name,
                    status="type_mismatch",
                    expected_type=cpp_type,
                    actual_type=blueprint_type,
                    suggestions=[f"Change C++ type to {self.blueprint_to_cpp_types.get(blueprint_type, blueprint_type)}"]
                )
        else:
            # Widget not found - look for similar names
            suggestions = self._find_similar_widget_names(variable_name, blueprint_widgets.keys())
            
            return BindingValidationResult(
                widget_name=variable_name,
                cpp_variable_name=variable_name,
                status="missing",
                expected_type=cpp_type,
                suggestions=suggestions
            )
    
    def _are_types_compatible(self, cpp_type: str, blueprint_type: str) -> bool:
        """Check if C++ type is compatible with blueprint widget type"""
        
        # Direct mapping check
        expected_blueprint_type = self.widget_type_mappings.get(cpp_type)
        if expected_blueprint_type == blueprint_type:
            return True
        
        # Handle inheritance and compatibility
        compatibility_map = {
            "UWidget": ["Button", "TextBlock", "Image", "ProgressBar", "Slider", "CheckBox"],
            "UPanelWidget": ["CanvasPanel", "VerticalBox", "HorizontalBox", "GridPanel", "Border"]
        }
        
        compatible_types = compatibility_map.get(cpp_type, [])
        return blueprint_type in compatible_types
    
    def _find_similar_widget_names(self, target_name: str, available_names: List[str]) -> List[str]:
        """Find similar widget names for suggestions"""
        
        suggestions = []
        target_lower = target_name.lower()
        
        # Look for partial matches
        for name in available_names:
            name_lower = name.lower()
            
            # Exact match (case insensitive)
            if target_lower == name_lower:
                suggestions.append(f"Use exact name: {name}")
                continue
            
            # Contains target or target contains name
            if target_lower in name_lower or name_lower in target_lower:
                suggestions.append(f"Similar name found: {name}")
            
            # Check for common prefixes/suffixes
            if (target_lower.startswith(name_lower[:3]) or 
                target_lower.endswith(name_lower[-3:]) or
                name_lower.startswith(target_lower[:3]) or
                name_lower.endswith(target_lower[-3:])):
                suggestions.append(f"Possible match: {name}")
        
        return suggestions[:3]  # Limit to top 3 suggestions
    
    def _generate_recommendations(self, binding_results: List[Dict[str, Any]], 
                                missing_widgets: List[str], extra_widgets: List[str],
                                blueprint_widgets: Dict[str, Dict[str, Any]]) -> List[str]:
        """Generate recommendations for fixing binding issues"""
        
        recommendations = []
        
        # Count issues
        failed_bindings = [r for r in binding_results if r["status"] != "found"]
        type_mismatches = [r for r in binding_results if r["status"] == "type_mismatch"]
        
        if failed_bindings:
            recommendations.append(f"Fix {len(failed_bindings)} binding issues")
        
        if type_mismatches:
            recommendations.append(f"Resolve {len(type_mismatches)} type mismatches")
        
        if missing_widgets:
            recommendations.append(f"Add {len(missing_widgets)} missing widgets to blueprint")
        
        if extra_widgets:
            recommendations.append(f"Consider binding {len(extra_widgets)} unbound widgets")
        
        # Specific recommendations
        if not binding_results and not missing_widgets:
            recommendations.append("All bindings are valid!")
        
        return recommendations
    
    def _determine_overall_status(self, binding_results: List[Dict[str, Any]], 
                                missing_widgets: List[str]) -> str:
        """Determine overall validation status"""
        
        if missing_widgets:
            return "error"
        
        failed_bindings = [r for r in binding_results if r["status"] != "found"]
        if failed_bindings:
            # Check if any are critical errors vs warnings
            critical_errors = [r for r in failed_bindings if r["status"] == "missing"]
            if critical_errors:
                return "error"
            else:
                return "warning"
        
        return "success"
    
    def _get_input_schema(self) -> Dict[str, Any]:
        """Get input schema for this agent"""
        return BINDING_VALIDATION_INPUT_SCHEMA
    
    def _get_output_schema(self) -> Dict[str, Any]:
        """Get output schema for this agent"""
        return BINDING_VALIDATION_OUTPUT_SCHEMA
