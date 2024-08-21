# Requests minimum CMake version
cmake_minimum_required(VERSION 3.26)

# Engine's application project
project(MeltdownApp)
# Engine's application name
set(MELTDOWN_APP meltdown_app)

# Utilizar C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Searches for the Meltdown Engine library
find_package(meltdown)

# Gathers source files (.cpp) to be compiled
file(GLOB_RECURSE APP_SRC_FILES "src/**.cpp")

# Creates a window only application (no terminal) for Windows on release builds
if(WIN32 AND CMAKE_BUILD_TYPE MATCHES "^(Release|MinSizeRel)$")
	add_executable(${MELTDOWN_APP} WIN32 ${APP_SRC_FILES})
	add_compile_definitions(${MELTDOWN_APP} PRIVATE MTD_APP_WINDOW_ONLY)
else()
	add_executable(${MELTDOWN_APP} ${APP_SRC_FILES})
endif()

# Configures the Meltdown library
target_link_libraries(${MELTDOWN_APP} PRIVATE meltdown)

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
