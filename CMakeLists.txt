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

# Gathers source files (.cpp) to be compiled
set(EXECUTABLE "meltdown_engine")
file(GLOB_RECURSE SRC_FILES "src/**.cpp")
add_executable(${EXECUTABLE} ${SRC_FILES})

# Defines macros for the current build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(${EXECUTABLE} PRIVATE MTD_DEBUG=2)
    message(STATUS "Building for debug...")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(${EXECUTABLE} PRIVATE MTD_DEBUG=1)
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
