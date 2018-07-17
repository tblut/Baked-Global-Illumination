# Glow Extras

Companion library for glow with convenience and helper functions.

## Usage

Add git submodule
```
git submodule add https://www.graphics.rwth-aachen.de:9000/Glow/glow-extras.git
git submodule update --init --recursive
```

Add to cmake (be sure to do that _after_ glow)
```
add_subdirectory(path/to/glow-extras)
```

Choose which libraries you need

```
# all
target_link_libraries(YourLib PUBLIC glow-extras)

# .. or single ones
target_link_libraries(YourLib PUBLIC glow-extras-camera)
target_link_libraries(YourLib PUBLIC glow-extras-geometry)
target_link_libraries(YourLib PUBLIC glow-extras-shader)
```

**CAUTION**: If you use extras that require dependencies, you have to add them manually before.
E.g. for `glow-extras-assimp`:

```
add_subdirectory(path/to/assimp) # https://graphics.rwth-aachen.de:9000/ptrettner/assimp-lean
add_subdirectory(path/to/glow-extras) # AFTER dependencies

target_link_libraries(YourLib PUBLIC glow-extras-assimp) # internally depends on assimp
```
glow-extras will disable (and tell you) all sub-libs that were disabled due to missing dependencies.

## Sub-libs

Currently supported sub-libs with their intent and dependencies.

* glow-extras-camera
* glow-extras-geometry
* glow-extras-shader
* glow-extras-assimp
* glow-extras-timing
* glow-extras-pipeline

### glow-extras-camera

Camera classes:
 * BaseCamera
 * StaticCamera
 * GenericCamera

### glow-extras-geometry

Template-heavy library for creating geometry (VertexArrays in particular)

Noteworthy classes:
 * Quad
 * Cube

### glow-extras-shader

*TODO*: Shader generation

### glow-extras-assimp

**DEPENDENCY**: assimp

Suggested submodule: https://www.graphics.rwth-aachen.de:9000/ptrettner/assimp-lean.git

*TODO*: Assimp importer

### glow-extras-timing

**DEPENDENCY**: aion

Suggested submodule: https://graphics.rwth-aachen.de:9000/ptrettner/aion

*TODO*: Aion timings and custom GPU timer

### glow-extras-pipeline

A pre-built Forward+ rendering pipeline with various configurable features.
