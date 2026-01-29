# Dynamit Designer

A Windows desktop application for visually designing 3D shapes using the dynamit_gl library's PolarBuilder API.

## Features

- **Real-time 3D Preview**: Interactive viewport with rotation and zoom controls
- **Shape Creation**: Create cones and cylinders using PolarBuilder
- **Builder Configuration**: Full control over formula, domain, sectors, slices, and flags
- **Transform Controls**: Position, rotation, and scale for each shape
- **Color Controls**: Inner and outer RGBA colors with color picker
- **Wireframe Mode**: Toggle wireframe rendering (F11)

## Building

### Prerequisites

1. Visual Studio 2022 with C++17 support
2. dynamit_gl library built and exported to `export/dynamitd_cpp/`
3. Third-party dependencies extracted (run `3rdparty/extract_libs.cmd`)

### Build Steps

1. Open `dynamit_designer.sln` in Visual Studio 2022
2. Set configuration to `Debug|x64`
3. Build the solution

Note: The dynamit_gl library must be built first. Open `dynamit_gl/dynamit_gl.sln` and build the `dynamit_gl` project to generate the export files.

## Usage

### Controls

| Key | Action |
|-----|--------|
| Arrow Keys | Rotate view |
| Mouse Drag | Rotate view (in viewport) |
| Scroll | Zoom in/out |
| F11 | Toggle wireframe mode |
| F9 | Toggle panel visibility |
| Escape | Exit application |

### Panels

#### Main Toolbar
- **New Cone**: Create a new cone shape
- **New Cylinder**: Create a new cylinder shape
- **Delete**: Delete the selected shape

#### Builder Settings
- **Formula**: Mathematical formula for radius (e.g., "1", "sin(theta)", "1 + 0.5*cos(4*theta)")
- **Domain Start/End**: Angular range in radians (typically 0 to 2*PI)
- **Sectors**: Number of angular divisions (3-128)
- **Slices**: Number of vertical divisions (1-64)
- **Smooth**: Enable smooth normals
- **Turbo**: Enable turbo mode (optimized generation)
- **Double Coated**: Render both sides
- **Reversed**: Reverse normals

#### Transform
- **Position X/Y/Z**: Shape position in world space
- **Rotation X/Y/Z**: Rotation in degrees
- **Scale X/Y/Z**: Scale factors

#### Colors
- **Outer Color**: RGBA color for outer surface
- **Inner Color**: RGBA color for inner surface (visible with Double Coated)
- **Pick...**: Open Windows color picker

## Project Structure

```
dynamit_designer/
├── dynamit_designer.sln      # Visual Studio solution
├── dynamit_designer.vcxproj  # Project file
├── main.cpp                  # Entry point, window init, render loop
├── DesignerApp.h/cpp         # Main application class
├── ShapeManager.h/cpp        # Shape instance management
├── dialogs/
│   ├── MainToolbar.h         # Shape selection buttons
│   ├── BuilderPanel.h        # PolarBuilder configuration
│   ├── TransformPanel.h      # Position/rotation/scale
│   └── ColorPanel.h          # Color RGBA controls
├── shaders/
│   ├── designer.vs           # Vertex shader
│   └── designer.fs           # Fragment shader
└── README.md                 # This file
```

## Dependencies

- **dynamit_gl.lib**: Core 3D graphics library (from `export/dynamitd_cpp/lib/`)
- **GLFW**: Window and input management
- **GLEW**: OpenGL extension loading
- **GLM**: Math library (matrices, vectors)

## Example Formulas

| Formula | Result |
|---------|--------|
| `1` | Simple cone/cylinder |
| `sin(theta)` | Sinusoidal surface |
| `1 + 0.3*cos(4*theta)` | 4-pointed star pattern |
| `theta / (2*pi)` | Spiral shape |
| `sqrt(sin(theta)^2)` | Absolute sine wave |

## License

Part of the 3D Dynamit project.
