# Requests minimum CMake version
cmake_minimum_required(VERSION 3.26)

# Defines project name
project("Meltdown Engine")

# Specifies C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Configures include directories and libraries
target_include_directories(${EXECUTABLE} PRIVATE ${INCLUDES})
target_link_libraries(${EXECUTABLE} PRIVATE ${LIBRARIES})

# Copies the selected files to the install location
install(TARGETS ${EXECUTABLE} DESTINATION .)
install(FILES $<TARGET_FILE:glfw> DESTINATION .)
