# BASELIBRARY

# Files comprising the target
file(GLOB sources "*.cpp")
file(GLOB headers "*.hpp")

file(GLOB src_exception "Exception/*.?pp")
file(GLOB src_graph "Graph/*.?pp")
file(GLOB src_grapheditor "GraphEditor/*.?pp")
file(GLOB src_logging "Logging/*.?pp")
file(GLOB src_memory "Memory/*.?pp")
file(GLOB src_platform "Platform/*.?pp")
file(GLOB src_platform_win32 "Platform/Win32/*.?pp")
file(GLOB src_serialization "Serialization/*.?pp")
file(GLOB src_jobsystem "JobSystem/*.?pp")

# Create target
add_library(BaseLibrary STATIC 
	${sources} ${headers}
	
	${src_exception}
	${src_graph}
	${src_grapheditor}
	${src_logging}
	${src_memory}
	${src_platform}
	${src_platform_win32}
	${src_serialization}
	${src_jobsystem}
)

# Filters
source_group("" FILES ${sources} ${headers})

source_group("Exception" FILES ${src_exception})
source_group("Graph" FILES ${src_graph})
source_group("GraphEditor" FILES ${src_grapheditor})
source_group("Logging" FILES ${src_logging})
source_group("Memory" FILES ${src_memory})
source_group("Platform" FILES ${src_platform})
source_group("Platform\\Win32" FILES ${src_platform_win32})
source_group("Serialization" FILES ${src_serialization})
source_group("JobSystem" FILES ${src_jobsystem})

# Dependencies
if (TARGET_PLATFORM_WINDOWS) 
	set(LINK_DBGHELP 1)
else()
	set(LINK_DBGHELP 0)
endif()

target_link_libraries(BaseLibrary
	$<IF:${LINK_DBGHELP},dbghelp,>
)
