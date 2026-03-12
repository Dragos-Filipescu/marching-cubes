cmake_minimum_required(VERSION 3.31)
project(marching-cubes LANGUAGES CXX)

# ==============================================================================
# CMake Policies
# ==============================================================================

if (MSVC)
  if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
  endif()
  # Debug uses EditAndContinue, others use ProgramDatabase (PDB)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
      "$<$<CONFIG:Debug>:EditAndContinue>$<$<NOT:$<CONFIG:Debug>>:ProgramDatabase>")
endif()

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
find_program(GLSLC_EXECUTABLE glslc HINTS "$ENV{VULKAN_SDK}/Bin" REQUIRED)

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
    $ENV{VULKAN_SDK}/Include
)

# C++ Standard
target_compile_features(marching-cubes PRIVATE cxx_std_23)

# Link libraries
target_link_libraries(marching-cubes PRIVATE
    Vulkan::Vulkan
    glfw
    glm
)

# Ensure shaders are compiled before main binary
add_dependencies(marching-cubes Shaders)

# ==============================================================================
# Assets Handling
# ==============================================================================

set(ASSETS_INPUT_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/assets")
file(MAKE_DIRECTORY "${ASSETS_OUTPUT_DIR}")

add_custom_command(
    TARGET marching-cubes PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${ASSETS_INPUT_DIR}
            ${ASSETS_OUTPUT_DIR}
)

# ==============================================================================
# Compiler Warnings and Debug Symbols
# ==============================================================================
set(CONSTEXPR_STEPS "5000000")

if (MSVC)
    target_compile_options(marching-cubes PRIVATE /W4 /permissive- /Zc:__cplusplus)

    # Debug (classic)
    target_compile_options(marching-cubes PRIVATE
        $<$<CONFIG:Debug>:/Zi /ZI /Od /Ob0 /Oy- /constexpr:steps ${CONSTEXPR_STEPS}>
    )
    target_link_options(marching-cubes PRIVATE
        $<$<CONFIG:Debug>:/DEBUG>
    )

    # RelWithDebInfo: optimized but debugger-friendly
    target_compile_options(marching-cubes PRIVATE
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /Zo /Ob1 /Oy- /d2Zi+ /constexpr:steps ${CONSTEXPR_STEPS}>
    )
    target_link_options(marching-cubes PRIVATE
        $<$<CONFIG:RelWithDebInfo>:/DEBUG /INCREMENTAL:NO>
    )

    # Release: optimized with symbols (for crash diagnostics)
    target_compile_options(marching-cubes PRIVATE
        $<$<CONFIG:Release>:/O2 /Zi /Ob2 /Oi /constexpr:steps ${CONSTEXPR_STEPS}>
    )
    target_link_options(marching-cubes PRIVATE
        $<$<CONFIG:Release>:/DEBUG /INCREMENTAL:NO>
    )

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(marching-cubes PRIVATE -Wall -Wextra -Wpedantic)

    # Debug
    target_compile_options(marching-cubes PRIVATE
        $<$<CONFIG:Debug>:-O0 -g3 -fno-omit-frame-pointer -fconstexpr-steps=${CONSTEXPR_STEPS}>
    )

    # RelWithDebInfo
    target_compile_options(marching-cubes PRIVATE
        $<$<CONFIG:RelWithDebInfo>:-O2 -g3 -fno-omit-frame-pointer -fconstexpr-steps=${CONSTEXPR_STEPS}>
    )

    # Release (with symbols for crash analysis)
    target_compile_options(marching-cubes PRIVATE
        $<$<CONFIG:Release>:-O3 -g1 -DNDEBUG -fno-omit-frame-pointer -fconstexpr-steps=${CONSTEXPR_STEPS}>
    )

elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(marching-cubes PRIVATE -Wall -Wextra -Wpedantic)

    # Debug
    target_compile_options(marching-cubes PRIVATE
        $<$<CONFIG:Debug>:-O0 -g3 -fno-omit-frame-pointer -fconstexpr-steps=${CONSTEXPR_STEPS}>
    )

    # RelWithDebInfo (optimized, still debuggable)
    target_compile_options(marching-cubes PRIVATE
        $<$<CONFIG:RelWithDebInfo>:-O2 -g3 -fno-omit-frame-pointer -fconstexpr-steps=${CONSTEXPR_STEPS}>
    )

    # Release (optimized, minimal debug info)
    target_compile_options(marching-cubes PRIVATE
        $<$<CONFIG:Release>:-O3 -g1 -DNDEBUG -fno-omit-frame-pointer -fconstexpr-steps=${CONSTEXPR_STEPS}>
    )
endif()
    
# Debug/Release Definitions
target_compile_definitions(marching-cubes PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release,RelWithDebInfo>:NDEBUG>
)

# ==============================================================================
# Installation
# ==============================================================================

install(TARGETS marching-cubes RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/ DESTINATION assets)
