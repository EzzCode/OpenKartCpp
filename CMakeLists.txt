cmake_minimum_required(VERSION 3.10)   # Selects the minimum version of CMake required to run this file
project(GFX-LAB VERSION 0.1.0)          # Here we select the project name and version


# Enable ccache if available for faster rebuilds
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()

# Here we select C++17 with all the standards required and all compiler-specific extensions disabled
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific configurations
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    
    # Only set runtime library for MSVC, not MinGW
    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC-specific optimizations
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-variable")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
        
        # Enable faster linking
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--build-id=none")
        endif()
    endif()
endif()

# These are the options we select for building GLFW as a library
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)        # Don't build Documentation
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)       # Don't build Tests
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)    # Don't build Examples
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)           # Don't build Installation Information
set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)     # Add variables to use High Performance Graphics Card if available
add_subdirectory(vendor/glfw)                       # Build the GLFW project to use later as a library

set(CMAKE_DISABLE_FIND_PACKAGE_PkgConfig TRUE CACHE BOOL "Disable PkgConfig search" FORCE)

# Disable FreeType build tests and examples to speed up build
set(FT_BUILD_TESTS OFF CACHE BOOL "Disable FreeType tests" FORCE)
set(FT_BUILD_EXAMPLES OFF CACHE BOOL "Disable FreeType examples" FORCE)


add_subdirectory(vendor/freetype-2.13.3)            # Build FreeType from vendor folder

# A variable with all the source files of GLAD
set(GLAD_SOURCE vendor/glad/src/gl.c)
# A variables with all the source files of Dear ImGui
set(IMGUI_SOURCES
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_widgets.cpp
        vendor/imgui/imgui_impl/imgui_impl_glfw.cpp
        vendor/imgui/imgui_impl/imgui_impl_opengl3.cpp
        )

# Combine all vendor source files together into a single variable
set(VENDOR_SOURCES ${GLAD_SOURCE} ${IMGUI_SOURCES})

# A variable with all our source files that are common between executable targets (examples)
set(COMMON_SOURCES
        source/common/application.hpp
        source/common/application.cpp
        source/common/input/keyboard.hpp
        source/common/input/mouse.hpp

        source/common/asset-loader.cpp
        source/common/asset-loader.hpp
        source/common/deserialize-utils.hpp
        
        source/common/shader/shader.hpp
        source/common/shader/shader.cpp

        source/common/mesh/vertex.hpp
        source/common/mesh/mesh.hpp
        source/common/mesh/mesh-utils.hpp
        source/common/mesh/mesh-utils.cpp

        source/common/texture/sampler.hpp
        source/common/texture/sampler.cpp
        source/common/texture/texture2d.hpp
        source/common/texture/texture-utils.hpp
        source/common/texture/texture-utils.cpp
        source/common/texture/screenshot.hpp
        source/common/texture/screenshot.cpp

        source/common/material/pipeline-state.hpp
        source/common/material/pipeline-state.cpp
        source/common/material/material.hpp
        source/common/material/material.cpp

        source/common/ecs/component.hpp
        source/common/ecs/transform.hpp
        source/common/ecs/transform.cpp
        source/common/ecs/entity.hpp
        source/common/ecs/entity.cpp
        source/common/ecs/world.hpp
        source/common/ecs/world.cpp

        source/common/components/camera.hpp
        source/common/components/camera.cpp
        source/common/components/mesh-renderer.hpp
        source/common/components/mesh-renderer.cpp
        source/common/components/free-camera-controller.hpp
        source/common/components/free-camera-controller.cpp
        source/common/components/light.hpp
        source/common/components/input.hpp
        source/common/components/input.cpp
        source/common/components/rigidbody.cpp
        source/common/components/rigidbody.hpp
        source/common/components/collider.cpp
        source/common/components/collider.hpp
        source/common/components/light.cpp        
        source/common/components/movement.hpp
        source/common/components/movement.cpp
        source/common/components/component-deserializer.hpp
        source/common/components/sound.hpp
        source/common/components/sound.cpp
        source/common/components/race.hpp
        source/common/components/race.cpp
        
        source/common/systems/miniaudio.h        
        source/common/systems/forward-renderer.hpp
        source/common/systems/forward-renderer.cpp
        source/common/systems/ColliderSystem.hpp
        source/common/systems/free-camera-controller.hpp
        source/common/systems/InputMovement.hpp
        source/common/systems/rigidbodySystem.hpp
        source/common/systems/movement.hpp
        source/common/systems/BulletDebugDrawer.hpp
        source/common/systems/BulletDebugDrawer.cpp
        source/common/systems/race-system.hpp
        source/common/systems/race-system.cpp
        source/common/systems/hud-system.hpp
        source/common/systems/hud-system.cpp
        # Text rendering is implemented in ForwardRenderer, these files don't exist
        # source/common/text/text-renderer.hpp
        # source/common/text/text-renderer.cpp
        # source/common/text/font.hpp
        # source/common/text/font.cpp
)

# Define the directories in which to search for the included headers
include_directories(
        source/common
        source/common/text
        vendor/glfw/include
        vendor/glad/include
        vendor/glm
        vendor/imgui
        vendor/imgui/imgui_impl
        vendor/utils
        vendor/bullet
        vendor/freetype-2.13.3/include
)

 
link_directories(${PROJECT_SOURCE_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
 
set(STATES_SOURCES
        source/states/play-state.hpp
        source/states/menu-state.hpp
        source/states/shader-test-state.hpp
        source/states/mesh-test-state.hpp
        source/states/transform-test-state.hpp
        source/states/pipeline-test-state.hpp
        source/states/texture-test-state.hpp
        source/states/sampler-test-state.hpp
        source/states/material-test-state.hpp
        source/states/entity-test-state.hpp
        source/states/renderer-test-state.hpp
        source/states/text-demo-state.hpp
)

# For each example, we add an executable target
# Each target compiles one example source file and the common & vendor source files
# Then we link GLFW with each target
add_executable(GAME_APPLICATION source/main.cpp ${STATES_SOURCES} ${COMMON_SOURCES} ${VENDOR_SOURCES})

# Additional include directories specific to the target
target_include_directories(GAME_APPLICATION PRIVATE
    vendor/imgui/imgui_impl
)

# Link libraries
target_link_libraries(GAME_APPLICATION 
    glfw
    freetype
)

# Add Bullet physics library directory
target_link_directories(GAME_APPLICATION PRIVATE ${CMAKE_SOURCE_DIR}/libs)

# Link Bullet libraries (without .a extension for CMake)
target_link_libraries(GAME_APPLICATION 
    BulletDynamics
    BulletCollision
    LinearMath
    BulletSoftBody
)

# Windows-specific libraries
if(WIN32)
    target_link_libraries(GAME_APPLICATION 
        opengl32
        gdi32
        user32
        kernel32
    )
endif()
