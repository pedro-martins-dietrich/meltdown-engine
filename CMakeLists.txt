# Requests minimum CMake version
cmake_minimum_required(VERSION 3.26)

# Defines project name
project("Meltdown Engine")

# Specifies C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Defines build type if undefined
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
    message(STATUS "No build type specified. Using \"Debug\"")
endif()

# Searches for required packages
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

# Sets include directories and libraries
set(
    INCLUDES
    ${Vulkan_INCLUDE_DIRS}
)
set(
    LIBRARIES
    glfw
    glm::glm
    Vulkan::Vulkan
)

# Sets resources directory
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")

# Sets shaders directories
set(SOURCE_SHADERS_DIR "${CMAKE_SOURCE_DIR}/src/Engine/Shaders")
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
        COMMENT "Compiled shader: ${SHADER_NAME}"
    )

    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()
add_custom_target(SHADERS DEPENDS ${SHADER_OUTPUTS})

# Gathers source files (.cpp) to be compiled
set(EXECUTABLE "meltdown_engine")
file(GLOB_RECURSE SRC_FILES "src/**.cpp")
add_executable(${EXECUTABLE} ${SRC_FILES})
add_dependencies(${EXECUTABLE} SHADERS)

# Define macros for the project
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(${EXECUTABLE} PRIVATE MTD_DEBUG=2)
    add_compile_definitions(${EXECUTABLE} PRIVATE MTD_RESOURCES_PATH="${RESOURCES_DIR}/")

    message(STATUS "Building for debug...")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(${EXECUTABLE} PRIVATE MTD_DEBUG=1)
    add_compile_definitions(${EXECUTABLE} PRIVATE MTD_RESOURCES_PATH="${RESOURCES_DIR}/")

    message(STATUS "Building for release with debug info...")
elseif(CMAKE_BUILD_TYPE MATCHES "^(Release|MinSizeRel)$")
    message(STATUS "Building for release...")
else()
    message(WARNING "Unknown build type: \"${CMAKE_BUILD_TYPE}\"")
endif()

# Configures include directories and libraries
target_include_directories(${EXECUTABLE} PRIVATE ${INCLUDES})
target_link_libraries(${EXECUTABLE} PRIVATE ${LIBRARIES})

# Copies the selected files to the install location
install(TARGETS ${EXECUTABLE} DESTINATION .)
install(FILES $<TARGET_FILE:glfw> DESTINATION .)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources" DESTINATION .)
