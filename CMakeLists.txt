cmake_minimum_required(VERSION 3.0.0)   # Selects the minimum version of CMake required to run this file
project(tears_engine VERSION 0.1.0)          # Here we select the project name and version

# Here we select C++17 with all the standards required and all compiler-specific extensions disabled
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# These are the options we select for building GLFW as a library
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)        # Don't build Documentation
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)       # Don't build Tests
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)    # Don't build Examples
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)           # Don't build Installation Information
set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)     # Add variables to use High Performance Graphics Card if available
add_subdirectory(vendor/glfw)                       # Build the GLFW project to use later as a library

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
        common/application.cpp
        common/shader.cpp
        common/mesh/mesh-utils.cpp
        common/texture/texture-utils.cpp
        common/texture/screenshot.cpp
        ECS/GameState/GameState.cpp
        ECS/Component/TransformComponent.cpp
        ECS/Component/CameraComponent.cpp
        ECS/Component/MeshRenderer.cpp
        ECS/Component/RenderState.cpp
        ECS/Entity/Entity.cpp common/texture/Texture2D.cpp common/texture/Texture2D.h common/texture/Sampler.cpp common/texture/Sampler.h ECS/Component/Material.cpp ECS/Component/Material.h ECS/Component/CameraControllerComponent.cpp)

# Define the directories in which to search for the included headers
include_directories(
        common
        vendor/glfw/include
        vendor/glad/include
        vendor/glm
        vendor/imgui
        vendor/utils
        ECS
)
add_executable(tears_engine main.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(tears_engine glfw)