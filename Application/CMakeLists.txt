# Requests minimum CMake version
cmake_minimum_required(VERSION 3.31)

# Engine's application project
project(MeltdownApp)
# Engine's application name
set(MELTDOWN_APP meltdown_app)

# Defines language version to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Searches for the Meltdown Engine library and others
find_package(meltdown)
find_package(imgui REQUIRED)

# Sets resources directory
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
# Sets shaders directories
set(SOURCE_SHADERS_DIR "src/Shaders")
set(COMPILED_SHADERS_DIR "${RESOURCES_DIR}/shaders")

# Searches shaders source code files and compiles them
file(
    GLOB_RECURSE SHADER_SRC_FILES
    "${SOURCE_SHADERS_DIR}/**.vert"
    "${SOURCE_SHADERS_DIR}/**.frag"
    "${SOURCE_SHADERS_DIR}/**.comp"
    "${SOURCE_SHADERS_DIR}/**.rgen"
    "${SOURCE_SHADERS_DIR}/**.rmiss"
    "${SOURCE_SHADERS_DIR}/**.rahit"
    "${SOURCE_SHADERS_DIR}/**.rchit"
    "${SOURCE_SHADERS_DIR}/**.rint"
    "${SOURCE_SHADERS_DIR}/**.rcall"
)
foreach(SHADER_SRC_FILE ${SHADER_SRC_FILES})
    get_filename_component(SHADER_NAME ${SHADER_SRC_FILE} NAME)
    set(SHADER_OUTPUT "${COMPILED_SHADERS_DIR}/${SHADER_NAME}.spv")

    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E remove ${SHADER_OUTPUT}
        COMMAND glslc --target-env=vulkan1.3 "${SHADER_SRC_FILE}" -o "${SHADER_OUTPUT}"
        DEPENDS ${SHADER_SRC_FILE}
        COMMENT "[MELTDOWN APP] Compiled shader: ${SHADER_NAME}"
    )
    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()
add_custom_target(SHADERS DEPENDS ${SHADER_OUTPUTS})

# Gathers source files (.cpp) to be compiled
file(GLOB_RECURSE APP_SRC_FILES "src/**.cpp")

# Creates a window only application (no terminal) for Windows on release builds
if(WIN32 AND CMAKE_BUILD_TYPE MATCHES "^(Release|MinSizeRel)$")
	add_executable(${MELTDOWN_APP} WIN32 ${APP_SRC_FILES})
	add_compile_definitions(${MELTDOWN_APP} PRIVATE MTD_APP_WINDOW_ONLY)
else()
	add_executable(${MELTDOWN_APP} ${APP_SRC_FILES})
endif()

# The application depends on the shaders compilation
add_dependencies(${MELTDOWN_APP} SHADERS)

# Configures the Meltdown library
target_link_libraries(${MELTDOWN_APP} PRIVATE meltdown imgui::imgui)

# Copies DLL files when necessary
if(WIN32 AND MTD_SHARED_LIB)
	add_custom_command(
		TARGET ${MELTDOWN_APP}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${MELTDOWN_APP}> $<TARGET_FILE_DIR:${MELTDOWN_APP}>
		COMMAND_EXPAND_LISTS
	)
endif()

# Copies the selected files to the install location
install(TARGETS ${MELTDOWN_APP} DESTINATION .)
