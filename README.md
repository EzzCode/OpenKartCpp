# CMPN205 Graphics Engine Project

A 3D graphics engine and racing game built with OpenGL, developed as part of the CMPN205 Computer Graphics course at Cairo University Faculty of Engineering (CUFE).

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [System Requirements](#system-requirements)
- [Building the Project](#building-the-project)
- [Running the Application](#running-the-application)
- [Project Structure](#project-structure)
- [Game Controls](#game-controls)
- [Technical Implementation](#technical-implementation)
- [Dependencies](#dependencies)
- [Legal and Licensing](#legal-and-licensing)
- [Contributing](#contributing)
- [Acknowledgments](#acknowledgments)

## Overview

This project is a complete 3D graphics engine implementation featuring a racing game. It demonstrates advanced computer graphics concepts including forward rendering, lighting systems, physics simulation, and post-processing effects. The engine is built from scratch using modern OpenGL and follows an Entity-Component-System (ECS) architecture.

## Features

### Graphics Engine
- **Forward Rendering Pipeline** with transparent object sorting
- **Lighting System** supporting multiple light types (directional, point, spot)
- **Sky Sphere Rendering** for immersive environments
- **Post-processing Effects** including vignette and chromatic aberration
- **Texture Mapping** with support for albedo, specular, and roughness maps
- **Mesh Rendering** with custom mesh loading and vertex array objects
- **Material System** supporting tinted and textured materials
- **Camera System** with free-look and racing camera modes

### Game Features
- **3D Racing Game** with checkpoint-based progression
- **Physics Simulation** using Bullet Physics engine
- **Collision Detection** for 3D obstacles and boundaries
- **Sound System** with spatial audio and race event sounds using miniaudio
- **Text Rendering** using freetype
- **HUD System** displaying lap times, position, and game stats
- **Menu System** with state management

### Technical Features
- **Entity-Component-System (ECS)** architecture
- **Scene Serialization/Deserialization** from JSON files
- **Debug Rendering** with Bullet Physics debug drawer
- **Performance Monitoring** with frame time tracking


## Building the Project

### Prerequisites
1. Install **CMake** (version 3.10 or higher)
2. Install a compatible C++17 compiler
3. Ensure OpenGL drivers are up to date

### Build Steps

#### Visual Studio Code
If using VS Code with CMake Tools extension:
1. Open the project folder in VS Code
2. Select your compiler kit when prompted
3. Use `Ctrl+Shift+P` → "CMake: Build"
4. The executable will be generated in the `bin` folder

## Running the Application

### Basic Execution
```bash
# Run with default configuration
./bin/GAME_APPLICATION.exe

# Run with specific configuration
./bin/GAME_APPLICATION.exe -c='config/app.jsonc'

# Run for specific number of frames (for testing)
./bin/GAME_APPLICATION.exe -c='config/app.jsonc' -f=100
```

### Available Configurations
- `config/app.jsonc` - Main racing game


## Project Structure

```
CMPN205 Project - Student Version/
├── assets/                 # Game assets
│   ├── models/            # 3D models and meshes
│   ├── textures/          # Texture files
│   ├── shaders/           # GLSL shader files
│   └── sounds/            # Audio files
├── config/                # Configuration files
├── source/                # Source code
│   ├── common/            # Engine core
│   │   ├── components/    # ECS components
│   │   ├── systems/       # ECS systems
│   │   ├── ecs/          # ECS framework
│   │   ├── mesh/         # Mesh handling
│   │   ├── texture/      # Texture management
│   │   ├── material/     # Material system
│   │   └── shader/       # Shader management
│   └── states/           # Application states
├── vendor/               # Third-party libraries
├── bin/                  # Compiled executables
└── build/               # Build artifacts
```

## Game Controls

### Racing Game
- **Arrow Keys**: Steering and acceleration
- **WASD + SHIFT**: Camera controls
- **ESC**: Return to menu
- **F12**: Take screenshot
- **Mouse**: Camera control (in free camera mode)

### Menu Navigation
- **Mouse**: Navigate menu options
- **Enter**: Select menu item
- **ESC**: Exit application

## Technical Implementation

### Rendering Pipeline
1. **Scene Setup**: Load entities and components from JSON
2. **Culling**: Frustum culling for performance optimization
3. **Sorting**: Transparent objects sorted back-to-front
4. **Rendering**: Forward rendering with lighting calculations
5. **Post-processing**: Apply screen-space effects
6. **HUD Rendering**: Overlay UI elements

### ECS Architecture
- **Entities**: Containers for components
- **Components**: Data-only structures (Transform, MeshRenderer, Light, etc.)
- **Systems**: Logic processors (ForwardRenderer, PhysicsSystem, RaceSystem, etc.)

### Physics Integration
- **Bullet Physics**: Rigid body dynamics and collision detection
- **Collision Shapes**: Box, sphere, and mesh colliders
- **Debug Visualization**: Real-time physics debug rendering

## Dependencies

### Core Libraries
- **GLFW**: Window management and input handling
- **GLAD**: OpenGL function loading
- **GLM**: Mathematics library for graphics
- **Dear ImGui**: Immediate mode GUI
- **nlohmann/json**: JSON parsing and serialization

### Game-Specific Libraries
- **Bullet Physics**: Physics simulation and collision detection
- **MiniAudio**: Audio playback and sound management
- **FreeType**: Font rendering and text display
- **stb_image**: Image loading and texture creation

### Build Tools
- **CMake**: Cross-platform build system
- **ccache**: Compiler cache for faster builds (optional)

## Legal and Licensing

### Project License
This project is developed for educational purposes as part of the CMPN205 Computer Graphics course at Cairo University Faculty of Engineering. The codebase is provided for academic use and learning.

**Important**: This is a student project and should not be used for commercial purposes without proper licensing of all dependencies.

### Third-Party Licenses

#### GLFW
- **License**: zlib/libpng License
- **Copyright**: Copyright © 2002-2006 Marcus Geelnard, 2006-2019 Camilla Löwy
- **Website**: https://www.glfw.org/

#### GLAD
- **License**: MIT License
- **Generated from**: OpenGL Registry

#### GLM
- **License**: MIT License or The Happy Bunny License
- **Copyright**: Copyright © 2005 - 2020 G-Truc Creation

#### Dear ImGui
- **License**: MIT License
- **Copyright**: Copyright © 2014-2023 Omar Cornut

#### Bullet Physics
- **License**: zlib License
- **Copyright**: Copyright © 2003-2023 Erwin Coumans

#### nlohmann/json
- **License**: MIT License
- **Copyright**: Copyright © 2013-2022 Niels Lohmann

#### MiniAudio
- **License**: MIT No Attribution or Public Domain
- **Copyright**: Copyright © 2023 David Reid

#### FreeType
- **License**: FreeType License (BSD-style)
- **Copyright**: Copyright © 2006-2023 David Turner, Robert Wilhelm, and Werner Lemberg

#### stb_image
- **License**: MIT License or Public Domain
- **Copyright**: Sean Barrett and contributors

### Assets and Resources
- All 3D models, textures, and audio files included in this project are either:
  - Used under educational fair use provisions
  - Created specifically for this educational project

## Nintendo Intellectual Property Notice

This project is an original student work and is **not affiliated with, endorsed by, or associated with Nintendo** or any of its subsidiaries. All Nintendo trademarks, characters, and intellectual property referenced or depicted in this project remain the property of Nintendo Co., Ltd.

Any Nintendo-related content (such as character names, likenesses, or assets) used in this project is for educational demonstration only and is not intended for commercial use. If you are a representative of Nintendo and have concerns about any content, please contact the project maintainers for prompt removal or modification.



## Acknowledgments

### Course Information
- **Course**: CMPN205 Computer Graphics
- **Institution**: Cairo University Faculty of Engineering (CUFE)
- **Semester**: Fall 2024

### Special Thanks
- Course instructors and teaching assistants for guidance and support
- The open-source community for providing excellent graphics libraries
- Fellow students for collaboration and knowledge sharing

---

**Note**: This project is primarily educational and demonstrates computer graphics programming concepts. The implementation prioritizes learning objectives over production-ready optimization.