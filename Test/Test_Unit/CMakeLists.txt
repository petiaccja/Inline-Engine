# TESTUNIT

# Files
set(sources main.cpp)
file(GLOB baselib "BaseLibrary/*.?pp")
file(GLOB gxeng "GraphicsEngine/*.?pp")

# Target
add_executable(Test_Unit ${sources} ${baselib} ${gxeng})

# Filters
source_group("" FILES ${sources})
source_group("BaseLibrary" FILES ${baselib})
source_group("GraphicsEngine" FILES ${gxeng})

# Dependencies
target_link_libraries(Test_Unit
	BaseLibrary
	GraphicsEngine_LL
)