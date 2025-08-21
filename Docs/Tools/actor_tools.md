# Unreal MCP Editor Tools

This document provides detailed information about the actor tools available in the Unreal MCP integration.

## Overview

Actor tools allow you to manipulate actors in the Unreal Engine scene.

## Actor Tools

### get_actors_in_level

Get a list of all actors in the current level.

**Parameters:**
- None

**Returns:**
- List of all actors with their properties

**Example:**
```json
{
  "command": "get_actors_in_level",
  "params": {}
}
```

### find_actors_by_name

Find actors in the current level by name pattern.

**Parameters:**
- `pattern` (string) - The name or partial name pattern to search for

**Returns:**
- List of matching actor names

**Example:**
```json
{
  "command": "find_actors_by_name",
  "params": {
    "pattern": "Cube"
  }
}
```

### create_actor

Create a new actor in the current level.

**Parameters:**
- `name` (string) - The name for the new actor (must be unique)
- `type` (string) - The type of actor to create (must be uppercase)
- `location` (array, optional) - [X, Y, Z] coordinates for the actor's position, defaults to [0, 0, 0]
- `rotation` (array, optional) - [Pitch, Yaw, Roll] values for the actor's rotation, defaults to [0, 0, 0]
- `scale` (array, optional) - [X, Y, Z] values for the actor's scale, defaults to [1, 1, 1]

**Returns:**
- Information about the created actor

**Example:**
```json
{
  "command": "create_actor",
  "params": {
    "name": "MyCube",
    "type": "CUBE",
    "location": [0, 0, 100],
    "rotation": [0, 45, 0],
    "scale": [2, 2, 2]
  }
}
```

### delete_actor

Delete an actor by name.

**Parameters:**
- `name` (string) - The name of the actor to delete

**Returns:**
- Result of the delete operation

**Example:**
```json
{
  "command": "delete_actor",
  "params": {
    "name": "MyCube"
  }
}
```

### set_actor_transform

Set the transform (location, rotation, scale) of an actor.

**Parameters:**
- `name` (string) - The name of the actor to modify
- `location` (array, optional) - [X, Y, Z] coordinates for the actor's position
- `rotation` (array, optional) - [Pitch, Yaw, Roll] values for the actor's rotation
- `scale` (array, optional) - [X, Y, Z] values for the actor's scale

**Returns:**
- Result of the transform operation

**Example:**
```json
{
  "command": "set_actor_transform",
  "params": {
    "name": "MyCube",
    "location": [100, 200, 300],
    "rotation": [0, 90, 0]
  }
}
```

### get_actor_properties

Get all properties of an actor.

**Parameters:**
- `name` (string) **REQUIRED** - The name of the actor

**Returns:**
- Object containing all actor properties

**Example:**
```json
{
  "command": "get_actor_properties",
  "params": {
    "name": "MyCube"
  }
}
```

### set_actor_property

Set a property on an actor.

**Parameters:**
- `name` (string) **REQUIRED** - Name of the actor
- `property_name` (string) **REQUIRED** - Name of the property to set
- `property_value` (string) **REQUIRED** - Value to set the property to (passed as string, converted internally)

**Returns:**
- Dict containing response information

**Example:**
```json
{
  "command": "set_actor_property",
  "params": {
    "name": "MyCube",
    "property_name": "bHidden",
    "property_value": "True"
  }
}
```

### set_light_property

Set a property on a light component.

**Parameters:**
- `name` (string) **REQUIRED** - Name of the light actor
- `property_name` (string) **REQUIRED** - Property to set, one of:
  - "Intensity": Brightness of the light (float)
  - "LightColor": Color of the light (array [R, G, B] with values 0-1)
  - "AttenuationRadius": How far the light reaches (float)
  - "SourceRadius": Size of the light source (float)
  - "SoftSourceRadius": Size of the soft light source border (float)
  - "CastShadows": Whether the light casts shadows (boolean)
- `property_value` (string) **REQUIRED** - Value to set the property to

**Returns:**
- Dict containing response information

**⚠️ Common Issues:**
- LightColor format issues: Use array format [1.0, 0.8, 0.6] for RGB values 0-1
- Property names are case-sensitive
- Some properties may not be available for all light types

**Examples:**
```json
// Set light intensity
{
  "command": "set_light_property",
  "params": {
    "name": "MyPointLight",
    "property_name": "Intensity",
    "property_value": "5000.0"
  }
}

// Set light attenuation radius
{
  "command": "set_light_property",
  "params": {
    "name": "MyPointLight",
    "property_name": "AttenuationRadius",
    "property_value": "500.0"
  }
}

// Enable shadow casting
{
  "command": "set_light_property",
  "params": {
    "name": "MyPointLight",
    "property_name": "CastShadows",
    "property_value": "true"
  }
}
```

### spawn_actor

Create a new basic Unreal Engine actor in the current level.

**Parameters:**
- `name` (string) **REQUIRED** - The name to give the new actor (must be unique)
- `type` (string) **REQUIRED** - The type of built-in actor to create. Supported types:
  - StaticMeshActor: Basic static mesh actor
  - PointLight: Point light source
  - SpotLight: Spot light source
  - DirectionalLight: Directional light source
  - CameraActor: Camera actor
- `location` (array, optional) - The [x, y, z] world location to spawn at
- `rotation` (array, optional) - The [pitch, yaw, roll] rotation in degrees

**Returns:**
- Dict containing the created actor's properties

**Examples:**
```json
// Spawn a point light at origin
{
  "command": "spawn_actor",
  "params": {
    "name": "MyLight",
    "type": "PointLight"
  }
}

// Spawn a static mesh at a specific location
{
  "command": "spawn_actor",
  "params": {
    "name": "MyCube",
    "type": "StaticMeshActor",
    "location": [100, 200, 50],
    "rotation": [0, 45, 0]
  }
}
```

### spawn_blueprint_actor

Spawn an actor from a Blueprint class in the current level.

**Parameters:**
- `blueprint_name` (string) **REQUIRED** - Path to the Blueprint to spawn from. Can be:
  - Absolute path: "/Game/Blueprints/BP_Character"
  - Relative path: "BP_Character" (will be prefixed with "/Game/Blueprints/")
- `actor_name` (string) **REQUIRED** - Name to give the spawned actor instance (must be unique)
- `location` (array, optional) - The [x, y, z] world location to spawn at
- `rotation` (array, optional) - The [pitch, yaw, roll] rotation in degrees

**Returns:**
- Dict containing the spawned actor's properties

**Examples:**
```json
// Spawn a blueprint actor with a relative path
{
  "command": "spawn_blueprint_actor",
  "params": {
    "blueprint_name": "BP_Character",
    "actor_name": "MyCharacter_1"
  }
}

// Spawn a blueprint actor with a full path
{
  "command": "spawn_blueprint_actor",
  "params": {
    "blueprint_name": "/Game/Characters/BP_Enemy",
    "actor_name": "Enemy_1",
    "location": [100, 200, 50],
    "rotation": [0, 45, 0]
  }
}
```

## Error Handling

All command responses include a "success" field indicating whether the operation succeeded, and an optional "message" field with details in case of failure.

```json
{
  "success": false,
  "message": "Actor 'MyCube' not found in the current level"
}
```

## Implementation Notes

- All numeric parameters for transforms (location, rotation, scale) must be provided as lists of 3 float values
- Actor types should be provided in uppercase
- The server maintains logging of all operations with detailed information and error messages
- All commands are executed through a connection to the Unreal Engine editor

## Type Reference

### Actor Types

Supported actor types for the `create_actor` command:

- `CUBE` - Static mesh cube
- `SPHERE` - Static mesh sphere
- `CYLINDER` - Static mesh cylinder
- `PLANE` - Static mesh plane
- `POINT_LIGHT` - Point light source
- `SPOT_LIGHT` - Spot light source
- `DIRECTIONAL_LIGHT` - Directional light source
- `CAMERA` - Camera actor
- `EMPTY` - Empty actor (container)

## Troubleshooting and Error Handling

### Common Errors and Solutions

**Error: "Actor 'X' not found in the current level"**
- **Cause**: Actor name doesn't exist or is misspelled
- **Solution**: Use find_actors_by_name to locate existing actors, check exact spelling

**Error: "Invalid parameters for command 'set_light_property'"**
- **Cause**: Incorrect property_value format, especially for LightColor
- **Solution**: Ensure proper data types - use string values for all property_value parameters

**Error: "Blueprint not found"**
- **Cause**: Incorrect Blueprint path in spawn_blueprint_actor
- **Solution**: Use full paths like "/Game/Blueprints/BP_Actor" or verify Blueprint exists

**Error: "Actor name already exists"**
- **Cause**: Trying to spawn actor with a name that's already taken
- **Solution**: Use unique names or delete existing actor first

### Best Practices

1. **Use find_actors_by_name first**: Check what actors exist before operations
2. **Use full Blueprint paths**: Specify complete paths for predictable behavior
3. **Check actor types**: Verify actor supports the property you're trying to set
4. **Use unique names**: Ensure actor names are unique when spawning
5. **Handle transform arrays**: Always provide 3-element arrays for location/rotation/scale

### Light Property Reference

**Intensity Values:**
- Indoor lights: 1000-5000
- Outdoor lights: 10000-50000
- Dramatic lighting: 100000+

**Color Values:**
- RGB values should be 0.0-1.0
- Common colors: [1.0, 1.0, 1.0] (white), [1.0, 0.8, 0.6] (warm), [0.6, 0.8, 1.0] (cool)

**Attenuation Radius:**
- Small rooms: 500-1000
- Large areas: 2000-5000
- Outdoor scenes: 10000+

### Actor Type Reference (Updated)

**Basic Actor Types (spawn_actor):**
- `StaticMeshActor` - Basic static mesh actor
- `PointLight` - Point light source
- `SpotLight` - Spot light source  
- `DirectionalLight` - Directional light source
- `CameraActor` - Camera actor

**Blueprint Actors (spawn_blueprint_actor):**
- Custom Blueprint classes created in the project
- Must specify full path or relative path from /Game/Blueprints/
- Can have custom components, logic, and properties

### Debugging Workflow

1. **List actors**: Use get_actors_in_level to see current scene state
2. **Find specific actors**: Use find_actors_by_name with patterns
3. **Check properties**: Use get_actor_properties to see current values
4. **Test changes**: Make small changes and verify results
5. **Handle errors**: Check response messages for specific error details