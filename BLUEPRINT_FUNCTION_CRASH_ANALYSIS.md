# Blueprint Custom Function Creation Crash Analysis

## Problem Summary
Creating custom Blueprint functions through the Unreal MCP system causes Unreal Engine to crash when attempting to open the Blueprint containing the custom function. The crash occurs with a `NAME_None` assertion failure in `BlueprintEditorUtils.cpp` at line 5834.

## Crash Details
**Assertion**: `(OldName != NAME_None) && (NewName != NAME_None)`
**File**: `D:\build\++UE5\Sync\Engine\Source\Editor\UnrealEd\Private\Kismet2\BlueprintEditorUtils.cpp`
**Line**: 5834
**Timing**: Crash occurs when opening the Blueprint in the editor, NOT during function creation

## Investigation Timeline

### Attempt 1: Original Implementation (CRASHED)
**Approach**: Manual creation of `UK2Node_FunctionEntry` and `UK2Node_FunctionResult` nodes with custom pins
**Problem**: Missing `AllocateDefaultPins()` calls and improper node initialization sequence
**Result**: Immediate crashes with NAME_None assertions

### Attempt 2: High-Level API Approach (COMPILATION FAILED)
**Approach**: Tried using `FKismetEditorUtilities` and `FBlueprintEditorUtils::CreateUserDefinedFunction`
**Problem**: These APIs don't exist in UE 5.6
**Result**: Compilation errors, approach abandoned

### Attempt 3: Comprehensive Validation (CRASHED)
**Approach**: Added extensive validation and proper node initialization sequence
- Function name validation 
- Parameter name validation (no spaces, hyphens, empty names)
- `AllocateDefaultPins()` calls before custom pin creation
- Proper node reconstruction sequence
- Graph refresh calls

**Result**: Still crashed with same NAME_None assertion despite all validation

### Attempt 4: Minimal Approach (NO CRASH BUT BROKEN)
**Approach**: Created only empty function graphs with no nodes
**Result**: 
- ✅ No crashes
- ❌ "Could not find a root node for the graph" error
- ❌ Functions completely non-functional

### Attempt 5: Basic Nodes Only (STILL CRASHED)
**Approach**: Create only basic `UK2Node_FunctionEntry` and `UK2Node_FunctionResult` nodes with default pins only
**Result**: Still crashes despite avoiding all custom pin creation

## Root Cause Analysis

### Core Issue
The fundamental problem appears to be **manual node creation incompatibility** with Unreal Engine's Blueprint system. The crash consistently occurs with:
- Any manual creation of `UK2Node_FunctionEntry` nodes
- Any manual creation of `UK2Node_FunctionResult` nodes  
- Even when following "proper" initialization sequences
- Even when avoiding all custom pin manipulation

### Key Technical Findings

1. **Function Creation Success**: The MCP tool successfully creates function graphs and reports success
2. **Crash on Blueprint Open**: The crash only occurs when opening the Blueprint in the editor
3. **NAME_None Assertion**: Always the same assertion failure related to name validation
4. **Validation Ineffective**: Even comprehensive name validation doesn't prevent the crashes
5. **Empty Graphs Work**: Only completely empty graphs (no nodes) don't crash

### Log Evidence
```
[2025.07.01-18.50.22:197][163]LogTemp: Created custom function 'ValidatedFunction' in blueprint 'BP_ValidationTest' with 2 inputs and 2 outputs
[2025.07.01-18.50.27:066][494]LogWindows: Error: Assertion failed: (OldName != NAME_None) && (NewName != NAME_None)
```

## Current Status: UNRESOLVED

### What Works
- ✅ Blueprint creation through MCP
- ✅ Basic function graph creation
- ✅ MCP communication and error handling

### What Doesn't Work  
- ❌ Any functional custom function creation
- ❌ Manual node creation (crashes)
- ❌ Custom pin creation (crashes)
- ❌ Opening Blueprints with created functions (crashes)

## Potential Solutions to Investigate

### 1. Alternative Blueprint Function Creation APIs
- Research UE 5.6-specific function creation methods
- Look for Blueprint compilation utilities that handle node creation
- Investigate Blueprint factory patterns

### 2. Deferred Node Creation
- Create graphs without nodes initially
- Use Blueprint editor callbacks to create nodes after Blueprint opens
- Hook into Blueprint compilation events

### 3. Template-Based Approach
- Create template Blueprints with pre-made functions
- Copy/clone function graphs from templates
- Modify existing function graphs instead of creating new ones

### 4. Blueprint Macro/Event Approach
- Use Blueprint macros instead of custom functions
- Create custom events instead of functions
- Investigate other Blueprint node types that might be more stable

### 5. Engine Source Investigation
- Examine how Unreal Editor creates functions through the UI
- Study the Blueprint editor's function creation workflow
- Look for internal APIs that properly handle node creation

## Tested Configurations

### Failed Approaches
- Manual UK2Node_FunctionEntry creation
- Manual UK2Node_FunctionResult creation  
- CreateUserDefinedPin() usage
- AllocateDefaultPins() + custom pins
- Comprehensive name validation
- Node reconstruction sequences
- Graph refresh operations

### Working Configurations
- Empty function graphs (but non-functional)
- Basic Blueprint creation without functions
- Standard MCP Blueprint operations (components, variables, etc.)

## Next Steps Recommendations

1. **Research Phase**: Study UE 5.6 Blueprint editor source code to understand proper function creation
2. **API Investigation**: Find alternative APIs for function creation that don't require manual node creation
3. **Template Strategy**: Consider using Blueprint templates or copying from existing functions
4. **Community Research**: Check Unreal Engine forums/documentation for 5.6-specific function creation methods
5. **Incremental Testing**: Try creating simpler Blueprint elements (macros, events) that might be more stable

## Files Modified
- `MCPGameProject/Plugins/UnrealMCP/Source/UnrealMCP/Private/Commands/UnrealMCPBlueprintCommands.cpp`
- Function: `HandleCreateCustomBlueprintFunction`

## Test Blueprints Created
- `BP_ValidationTest` (crashed)
- `BP_FreshTest` (crashed) 
- `BP_SafeTest` (crashed)

## Conclusion
Manual Blueprint function creation through low-level node APIs appears to be fundamentally incompatible with UE 5.6's Blueprint system. The issue persists regardless of validation, proper initialization sequences, or simplified approaches. A completely different strategy is needed that avoids manual node creation entirely. 