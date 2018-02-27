# ROOT PROJECT FILE

cmake_minimum_required(VERSION 3.10)

# Declare project
project(Inline-Engine)

# Project uses C and C++
enable_language(C)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

# Set platform-specific properties
include(GetPlatform.cmake)
include(Paths.cmake)

if (TARGET_COMPILER_MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

# Add additional folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(Engine)
add_subdirectory(Test)

add_custom_target(Files CMakeLists.txt GetPlatform.cmake)
SET_TARGET_PROPERTIES (Files PROPERTIES FOLDER Files)