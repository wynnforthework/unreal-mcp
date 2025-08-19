"""
Data Schemas for Agent Communication

Defines the JSON schemas and data structures used for communication
between agents in the UMG generation workflow.
"""

from typing import Dict, Any, List, Optional
from dataclasses import dataclass
from enum import Enum


class UIComponentType(Enum):
    """Supported UMG component types"""
    CANVAS_PANEL = "CanvasPanel"
    VERTICAL_BOX = "VerticalBox"
    HORIZONTAL_BOX = "HorizontalBox"
    GRID_PANEL = "GridPanel"
    UNIFORM_GRID_PANEL = "UniformGridPanel"
    SCROLL_BOX = "ScrollBox"
    BORDER = "Border"
    BUTTON = "Button"
    TEXT_BLOCK = "TextBlock"
    EDITABLE_TEXT = "EditableText"
    EDITABLE_TEXT_BOX = "EditableTextBox"
    IMAGE = "Image"
    PROGRESS_BAR = "ProgressBar"
    SLIDER = "Slider"
    CHECK_BOX = "CheckBox"
    COMBO_BOX_STRING = "ComboBoxString"
    LIST_VIEW = "ListView"
    TREE_VIEW = "TreeView"
    WIDGET_SWITCHER = "WidgetSwitcher"
    SPACER = "Spacer"


class BindingType(Enum):
    """Types of widget bindings"""
    BIND_WIDGET = "BindWidget"
    BIND_WIDGET_OPTIONAL = "BindWidgetOptional"
    BIND_WIDGET_ANIM = "BindWidgetAnim"


# JSON Schemas for agent communication

# 1. UI Requirements Parser Agent Input/Output
UI_REQUIREMENTS_INPUT_SCHEMA = {
    "type": "object",
    "properties": {
        "natural_language_request": {
            "type": "string",
            "description": "Natural language description of UI requirements"
        },
        "context": {
            "type": "object",
            "properties": {
                "project_type": {"type": "string"},
                "target_platform": {"type": "string"},
                "style_preferences": {"type": "object"}
            }
        }
    },
    "required": ["natural_language_request"]
}

UI_REQUIREMENTS_OUTPUT_SCHEMA = {
    "type": "object",
    "properties": {
        "ui_specification": {
            "type": "object",
            "properties": {
                "widget_name": {"type": "string"},
                "description": {"type": "string"},
                "components": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "id": {"type": "string"},
                            "type": {"type": "string"},
                            "label": {"type": "string"},
                            "description": {"type": "string"},
                            "position": {
                                "type": "object",
                                "properties": {
                                    "x": {"type": "number"},
                                    "y": {"type": "number"},
                                    "anchor": {"type": "string"}
                                }
                            },
                            "size": {
                                "type": "object",
                                "properties": {
                                    "width": {"type": "number"},
                                    "height": {"type": "number"},
                                    "auto_size": {"type": "boolean"}
                                }
                            },
                            "properties": {"type": "object"},
                            "parent_id": {"type": "string"},
                            "children": {"type": "array", "items": {"type": "string"}}
                        },
                        "required": ["id", "type", "label"]
                    }
                },
                "layout_type": {"type": "string"},
                "interactions": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "component_id": {"type": "string"},
                            "event_type": {"type": "string"},
                            "action": {"type": "string"}
                        }
                    }
                }
            },
            "required": ["widget_name", "components"]
        },
        "confidence": {"type": "number", "minimum": 0, "maximum": 1},
        "parsing_metadata": {"type": "object"}
    },
    "required": ["ui_specification"]
}

# 2. Design Translation Agent Input/Output
DESIGN_TRANSLATION_INPUT_SCHEMA = UI_REQUIREMENTS_OUTPUT_SCHEMA

DESIGN_TRANSLATION_OUTPUT_SCHEMA = {
    "type": "object",
    "properties": {
        "umg_widget_definition": {
            "type": "object",
            "properties": {
                "widget_name": {"type": "string"},
                "parent_class": {"type": "string", "default": "UserWidget"},
                "path": {"type": "string", "default": "/Game/Widgets"},
                "root_component": {
                    "type": "object",
                    "properties": {
                        "name": {"type": "string"},
                        "type": {"type": "string", "enum": [e.value for e in UIComponentType]},
                        "position": {
                            "type": "object",
                            "properties": {
                                "x": {"type": "number"},
                                "y": {"type": "number"}
                            }
                        },
                        "size": {
                            "type": "object",
                            "properties": {
                                "width": {"type": "number"},
                                "height": {"type": "number"}
                            }
                        },
                        "properties": {"type": "object"},
                        "children": {
                            "type": "array",
                            "items": {"$ref": "#/properties/umg_widget_definition/properties/root_component"}
                        }
                    },
                    "required": ["name", "type"]
                },
                "cpp_bindings": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "widget_name": {"type": "string"},
                            "cpp_variable_name": {"type": "string"},
                            "widget_type": {"type": "string"},
                            "binding_type": {"type": "string", "enum": [e.value for e in BindingType]},
                            "is_optional": {"type": "boolean"}
                        },
                        "required": ["widget_name", "cpp_variable_name", "widget_type"]
                    }
                },
                "event_bindings": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "widget_name": {"type": "string"},
                            "event_type": {"type": "string"},
                            "function_name": {"type": "string"}
                        }
                    }
                }
            },
            "required": ["widget_name", "root_component"]
        },
        "validation_results": {
            "type": "object",
            "properties": {
                "naming_validation": {"type": "boolean"},
                "hierarchy_validation": {"type": "boolean"},
                "binding_validation": {"type": "boolean"},
                "warnings": {"type": "array", "items": {"type": "string"}},
                "errors": {"type": "array", "items": {"type": "string"}}
            }
        }
    },
    "required": ["umg_widget_definition"]
}

# 3. UE Editor Execution Agent Input/Output
UE_EXECUTION_INPUT_SCHEMA = DESIGN_TRANSLATION_OUTPUT_SCHEMA

UE_EXECUTION_OUTPUT_SCHEMA = {
    "type": "object",
    "properties": {
        "execution_result": {
            "type": "object",
            "properties": {
                "widget_blueprint_path": {"type": "string"},
                "created_components": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "name": {"type": "string"},
                            "type": {"type": "string"},
                            "success": {"type": "boolean"},
                            "error_message": {"type": "string"}
                        }
                    }
                },
                "event_bindings_created": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "widget_name": {"type": "string"},
                            "event_type": {"type": "string"},
                            "function_name": {"type": "string"},
                            "success": {"type": "boolean"}
                        }
                    }
                },
                "compilation_status": {"type": "string"},
                "compilation_errors": {"type": "array", "items": {"type": "string"}}
            },
            "required": ["widget_blueprint_path"]
        },
        "performance_metrics": {
            "type": "object",
            "properties": {
                "creation_time": {"type": "number"},
                "component_count": {"type": "integer"},
                "binding_count": {"type": "integer"}
            }
        }
    },
    "required": ["execution_result"]
}

# 4. Binding Validation Agent Input/Output
BINDING_VALIDATION_INPUT_SCHEMA = {
    "type": "object",
    "properties": {
        "widget_blueprint_path": {"type": "string"},
        "cpp_header_path": {"type": "string"},
        "expected_bindings": {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "widget_name": {"type": "string"},
                    "cpp_variable_name": {"type": "string"},
                    "widget_type": {"type": "string"},
                    "binding_type": {"type": "string"}
                }
            }
        }
    },
    "required": ["widget_blueprint_path"]
}

BINDING_VALIDATION_OUTPUT_SCHEMA = {
    "type": "object",
    "properties": {
        "validation_report": {
            "type": "object",
            "properties": {
                "overall_status": {"type": "string", "enum": ["success", "warning", "error"]},
                "binding_results": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "widget_name": {"type": "string"},
                            "cpp_variable_name": {"type": "string"},
                            "status": {"type": "string", "enum": ["found", "missing", "type_mismatch", "name_mismatch"]},
                            "expected_type": {"type": "string"},
                            "actual_type": {"type": "string"},
                            "suggestions": {"type": "array", "items": {"type": "string"}}
                        }
                    }
                },
                "missing_widgets": {"type": "array", "items": {"type": "string"}},
                "extra_widgets": {"type": "array", "items": {"type": "string"}},
                "recommendations": {"type": "array", "items": {"type": "string"}}
            },
            "required": ["overall_status", "binding_results"]
        }
    },
    "required": ["validation_report"]
}
