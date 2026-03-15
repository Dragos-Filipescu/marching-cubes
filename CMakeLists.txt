cmake_minimum_required(VERSION 3.21)
project(marching-cubes LANGUAGES CXX)

# ==============================================================================
# Build Type Configuration
# ==============================================================================

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the build type" FORCE)
endif()

# ==============================================================================
# Dependencies
# ==============================================================================

# Vulkan SDK
find_package(Vulkan REQUIRED)

# GLSLC (Shader Compiler)
find_program(GLSLC_EXECUTABLE glslc HINTS "$ENV{VULKAN_SDK}/bin" REQUIRED)

# External dependencies
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

# ==============================================================================
# Shader Compilation
# ==============================================================================

file(GLOB SHADER_SOURCES "${CMAKE_SOURCE_DIR}/shaders/*shader.*")
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY "${SHADER_OUTPUT_DIR}")

set(SHADER_OUTPUTS "")
foreach(SHADER IN LISTS SHADER_SOURCES)
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV "${SHADER_OUTPUT_DIR}/${FILE_NAME}.spv")
    list(APPEND SHADER_OUTPUTS "${SPIRV}")

    add_custom_command(
        OUTPUT "${SPIRV}"
        COMMAND ${GLSLC_EXECUTABLE} "${SHADER}" -o "${SPIRV}"
        DEPENDS "${SHADER}"
        COMMENT "Compiling shader: ${FILE_NAME} => ${SPIRV}"
        VERBATIM
    )
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SHADER_OUTPUTS})

# ==============================================================================
# Source Files
# ==============================================================================

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    include/*.hpp
    include/*.h
)

# ==============================================================================
# Executable Target
# ==============================================================================

message(STATUS "Sources: ${SOURCES}")
add_executable(marching-cubes ${SOURCES})

# ==============================================================================
# Include Directories
# ==============================================================================

target_include_directories(marching-cubes PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/external/tinyobj
)

# C++ Standard
target_compile_features(marching-cubes PRIVATE cxx_std_26)

# Link libraries
target_link_libraries(marching-cubes PRIVATE
    Vulkan::Vulkan
    glfw
    glm
)

# Ensure shaders are compiled before main binary
add_dependencies(marching-cubes Shaders)

# ==============================================================================
# Compiler Warnings and Debug Symbols
# ==============================================================================
set(CONSTEXPR_STEPS "5000000")

target_compile_options(marching-cubes PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(marching-cubes PRIVATE -stdlib=libstdc++)
    target_link_options(marching-cubes PRIVATE -stdlib=libstdc++)
    set(CONSTEXPR_STEPS_FLAG "-fconstexpr-steps=${CONSTEXPR_STEPS}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CONSTEXPR_STEPS_FLAG "-fconstexpr-ops-limit=${CONSTEXPR_STEPS}")
endif()

# Debug
target_compile_options(marching-cubes PRIVATE
    $<$<CONFIG:Debug>:-O0 -g3 -fno-omit-frame-pointer ${CONSTEXPR_STEPS_FLAG}>
)
# RelWithDebInfo
target_compile_options(marching-cubes PRIVATE
    $<$<CONFIG:RelWithDebInfo>:-O2 -g3 -fno-omit-frame-pointer ${CONSTEXPR_STEPS_FLAG}>
)
# Release
target_compile_options(marching-cubes PRIVATE
    $<$<CONFIG:Release>:-O3 -g1 -DNDEBUG -fno-omit-frame-pointer ${CONSTEXPR_STEPS_FLAG}>
)

# Debug/Release Definitions
target_compile_definitions(marching-cubes PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release,RelWithDebInfo>:NDEBUG>
)

# ==============================================================================
# Installation
# ==============================================================================

install(TARGETS marching-cubes RUNTIME DESTINATION bin)
