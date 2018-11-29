# TESTUNIT

# Files
set(sources main.cpp)
file(GLOB baselib "BaseLibrary/*.?pp")
file(GLOB gxeng "GraphicsEngine/*.?pp")
file(GLOB guieng "GuiEngine/*.?pp")

# Target
add_executable(Test_Unit ${sources} ${baselib} ${gxeng} ${guieng})

# Filters
source_group("" FILES ${sources})
source_group("BaseLibrary" FILES ${baselib})
source_group("GraphicsEngine" FILES ${gxeng})
source_group("GuiEngine" FILES ${guieng})

# Dependencies
target_link_libraries(Test_Unit
	BaseLibrary
	GuiEngine
	GraphicsEngine_LL
)