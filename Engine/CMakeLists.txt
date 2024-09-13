# Requests minimum CMake version
cmake_minimum_required(VERSION 3.26)

# Meltdown Engine library project
project(Meltdown)

# Meltdown Engine library name
set(MELTDOWN_LIB meltdown)
# Library targets and configuration files
set(MTD_TARGETS "meltdownTargets")
set(MTD_CONFIG "meltdownConfig.cmake")

# Sets compilation with C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Defines build type if undefined
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
    message(STATUS "[MELTDOWN] No build type specified. Using \"Debug\".")
endif()

# Searches for required packages
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(imgui REQUIRED)
find_package(Stb REQUIRED)

# Sets include directories and libraries
set(
    MTD_INCLUDES
    ${Stb_INCLUDE_DIRS}
    ${Vulkan_INCLUDE_DIRS}
)
set(
    MTD_LIBRARIES
    glfw
    glm::glm
    nlohmann_json::nlohmann_json
    imgui::imgui
    Vulkan::Vulkan
)

# Sets resources directory
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
# Sets shaders directories
set(SOURCE_SHADERS_DIR "Shaders")
set(COMPILED_SHADERS_DIR "${RESOURCES_DIR}/shaders")

# Searches shaders source code files and compiles them
file(GLOB_RECURSE SHADER_SRC_FILES "${SOURCE_SHADERS_DIR}/**.vert" "${SOURCE_SHADERS_DIR}/**.frag")
foreach(SHADER_SRC_FILE ${SHADER_SRC_FILES})
    get_filename_component(SHADER_NAME ${SHADER_SRC_FILE} NAME)
    set(SHADER_OUTPUT "${COMPILED_SHADERS_DIR}/${SHADER_NAME}.spv")

    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E remove ${SHADER_OUTPUT}
        COMMAND glslc "${SHADER_SRC_FILE}" -o "${SHADER_OUTPUT}"
        DEPENDS ${SHADER_SRC_FILE}
        COMMENT "[MELTDOWN] Compiled shader: ${SHADER_NAME}"
    )

    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()
add_custom_target(SHADERS DEPENDS ${SHADER_OUTPUTS})

# Gathers source files (.cpp) to be compiled
file(GLOB_RECURSE SRC_FILES "src/**.cpp")

# Makes the engine a shared library if MTD_SHARED_LIBRARY is defined, otherwise, makes it a static library
if(MTD_SHARED_LIB)
    if(WIN32)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
        message(STATUS "[MELTDOWN] Exporting all DLL symbols for Windows.")
    endif()

    add_library(${MELTDOWN_LIB} SHARED ${SRC_FILES})

    add_compile_definitions(${MELTDOWN_LIB} PUBLIC MTD_SHARED)
    add_compile_definitions(${MELTDOWN_LIB} PRIVATE MTD_EXPORTS)
    message(STATUS "[MELTDOWN] Creating Meltdown Engine as a shared library...")
else()
    add_library(${MELTDOWN_LIB} STATIC ${SRC_FILES})

    message(STATUS "[MELTDOWN] Creating Meltdown Engine as a static library...")
endif()

# The library depends on the shaders compilation
add_dependencies(${MELTDOWN_LIB} SHADERS)

# Defines macros for the project
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(${MELTDOWN_LIB} PRIVATE MTD_DEBUG=2)
    add_compile_definitions(${MELTDOWN_LIB} PRIVATE MTD_RESOURCES_PATH="${RESOURCES_DIR}/")

    message(STATUS "[MELTDOWN] Building for debug...")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(${MELTDOWN_LIB} PRIVATE MTD_DEBUG=1)
    add_compile_definitions(${MELTDOWN_LIB} PRIVATE MTD_RESOURCES_PATH="${RESOURCES_DIR}/")

    message(STATUS "[MELTDOWN] Building for release with debug info...")
elseif(CMAKE_BUILD_TYPE MATCHES "^(Release|MinSizeRel)$")
    message(STATUS "[MELTDOWN] Building for release...")
else()
    message(WARNING "[MELTDOWN] Unknown build type: \"${CMAKE_BUILD_TYPE}\"")
endif()

# Configures the pre-compiled headers
set(MTD_PCH_DIR "pch")
target_precompile_headers(${MELTDOWN_LIB} PRIVATE "${MTD_PCH_DIR}/pch.hpp")

# Library interface include directory
set(MTD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(MTD_INSTALL_INCLUDE_DIR "Engine/include")

# Configures include directories and libraries
target_include_directories(
    ${MELTDOWN_LIB}
    PRIVATE ${MTD_INCLUDES} ${MTD_PCH_DIR}
    PUBLIC $<BUILD_INTERFACE:${MTD_INCLUDE_DIR}>
    PUBLIC $<INSTALL_INTERFACE:${MTD_INSTALL_INCLUDE_DIR}>
)
target_link_libraries(${MELTDOWN_LIB} PRIVATE ${MTD_LIBRARIES})

# Exports the Meltdown library
export(TARGETS ${MELTDOWN_LIB} FILE "${MTD_TARGETS}.cmake")
set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(PACKAGE ${MELTDOWN_LIB})

# Copies the selected files to the install location
install(
    TARGETS ${MELTDOWN_LIB}
    EXPORT ${MTD_TARGETS}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY "${MTD_INCLUDE_DIR}/" DESTINATION "include")
install(
    EXPORT ${MTD_TARGETS}
    FILE ${MTD_CONFIG}
    NAMESPACE mtd::
    DESTINATION "lib/cmake/meltdown"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${MTD_CONFIG}" DESTINATION "lib/cmake/meltdown")
install(DIRECTORY ${RESOURCES_DIR} DESTINATION .)
install(FILES $<TARGET_FILE:glfw> DESTINATION .)
if(MTD_SHARED_LIB)
    install(FILES $<TARGET_FILE:${MELTDOWN_LIB}> DESTINATION .)
endif()

# Exports the library targets
export(
    EXPORT ${MTD_TARGETS}
    FILE "${CMAKE_BINARY_DIR}/Engine/${MTD_CONFIG}"
    NAMESPACE mtd::
)

# Includes CPack for posterior installation
include(CPack)
