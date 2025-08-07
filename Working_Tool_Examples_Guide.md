# Working Tool Examples Guide

## Overview

This comprehensive guide provides working examples for all 63 MCP tools across 7 categories. Each example includes:
- Complete parameter syntax
- Expected results
- Common use cases
- Troubleshooting tips

## Table of Contents

1. [Project Management Tools (11 tools)](#project-management-tools)
2. [Editor Tools (10 tools)](#editor-tools)
3. [Blueprint System Tools (14 tools)](#blueprint-system-tools)
4. [Node System Tools (4 tools)](#node-system-tools)
5. [Blueprint Action Tools (6 tools)](#blueprint-action-tools)
6. [UMG System Tools (11 tools)](#umg-system-tools)
7. [DataTable System Tools (7 tools)](#datatable-system-tools)

---

## Project Management Tools

### 1. mcp_projectMCP_create_input_mapping
**Purpose**: Create legacy input action bindings

```bash
# Basic input mapping
mcp_projectMCP_create_input_mapping(
    action_name="Jump",
    key="SpaceBar"
)

# Input mapping with modifiers
mcp_projectMCP_create_input_mapping(
    action_name="QuickSave",
    key="F5",
    input_type="Action"
)
```

**Expected Result**: Input mapping created and registered
**Common Issues**: Ensure key names match UE conventions (SpaceBar, not Space)

### 2. mcp_projectMCP_create_enhanced_input_action
**Purpose**: Create Enhanced Input Action assets

```bash
# Digital action (button press)
mcp_projectMCP_create_enhanced_input_action(
    action_name="Jump",
    value_type="Digital",
    description="Player jump action"
)

# Analog action (trigger/stick)
mcp_projectMCP_create_enhanced_input_action(
    action_name="Move",
    value_type="Axis2D",
    path="/Game/Input/Actions"
)
```

**Expected Result**: IA_ActionName asset created in specified path
**Common Issues**: Use correct value_type (Digital, Analog, Axis2D, Axis3D)