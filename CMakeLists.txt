# ROOT PROJECT FILE

cmake_minimum_required(VERSION 3.10)

# Declare project
project(Inline-Engine)


# Project uses C and C++
enable_language(C)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)


# Configurations
set(CMAKE_CONFIGURATION_TYPES Debug RelWithDebInfo CACHE TYPE INTERNAL FORCE)
set(DEBUG_CONFIGURATIONS Debug)


# Set platform-specific properties
include(GetPlatform.cmake)
include(Paths.cmake)
include(CopyRuntime.cmake)
include(Utils.cmake)
include(WholeArchive.cmake)

if (TARGET_COMPILER_MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP /await /permissive-")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4638") # Ignore XML doc not found reference
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5033") # Register keyword is now deprecated but retarded Bullet uses it
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250") # Inheritence via dominance
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4099") # Missing PDB for libraries
	add_definitions("-D_ENABLE_EXTENDED_ALIGNED_STORAGE")
	# set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS} /PROFILE")
elseif (TARGET_COMPILER_CLANG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fcoroutines-ts /std:c++17")
endif()

if (TARGET_PLATFORM_WINDOWS)
	if (defined ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
		if (${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION} VERSION_LESS "10.0.16299.0")
			set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION 10.0.16299.0)
		endif()
	endif()
endif()


# Enable filter grouping of projects inside solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


# Properties applied to all targets
include_directories("${CMAKE_CURRENT_LIST_DIR}/Engine")
include_directories("${EXTERNALS_INCLUDE}/Bullet") # because Bullet is retarded...


# Add additional folders
add_subdirectory(Engine)
add_subdirectory(Test)
add_subdirectory(Executables)


# Add custom target containing global files
add_custom_target(Files SOURCES 
	CMakeLists.txt
	Paths.cmake
	GetPlatform.cmake
	CopyRuntime.cmake
	Utils.cmake
	WholeArchive.cmake)
set_target_properties(Files PROPERTIES FOLDER Files)

# Add Mathter.natvis to all targets
get_all_targets(all_targets ${CMAKE_SOURCE_DIR})
foreach (tar IN ITEMS ${all_targets})
	get_target_property(tar_type ${tar} TYPE)
	if (tar_type MATCHES "(STATIC_LIBRARY|MODULE_LIBRARY|SHARED_LIBRARY|EXECUTABLE)")
		target_sources(${tar} PRIVATE "${EXTERNALS_INCLUDE}/Mathter/Mathter.natvis")
	endif()
endforeach()
source_group("" FILES "${EXTERNALS_INCLUDE}/Mathter/Mathter.natvis")

# Enable whole-archive on marked projects.
set_whole_archive_linkage(${CMAKE_SOURCE_DIR})

# Put projects in their folders
set_target_properties(BaseLibrary PROPERTIES FOLDER Modules)
if (TARGET_PLATFORM_WINDOWS)
	set_target_properties(GraphicsApi_D3D12 PROPERTIES FOLDER Modules)
endif()
set_target_properties(GraphicsEngine_LL PROPERTIES FOLDER Modules)
set_target_properties(GraphicsPipeline PROPERTIES FOLDER Modules)
set_target_properties(AssetLibrary PROPERTIES FOLDER Modules)
set_target_properties(GuiEngine PROPERTIES FOLDER Modules)
#set_target_properties(NetworkEngine PROPERTIES FOLDER Modules)
#set_target_properties(NetworkApi PROPERTIES FOLDER Modules)
set_target_properties(PhysicsEngine_Bullet PROPERTIES FOLDER Modules)

set_target_properties(Test_General PROPERTIES FOLDER Test)
set_target_properties(QC_Simulator PROPERTIES FOLDER Test)
set_target_properties(Test_Unit PROPERTIES FOLDER Test)
set_target_properties(Test_Physics PROPERTIES FOLDER Test)

set_target_properties(NodeEditor PROPERTIES FOLDER Executables)
set_target_properties(NodeEditor_v2 PROPERTIES FOLDER Executables)


