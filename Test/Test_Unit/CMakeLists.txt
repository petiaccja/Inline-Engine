# TESTUNIT

# Files
set(sources main.cpp)
file(GLOB baselib "BaseLibrary/*.?pp")
file(GLOB gxeng "GraphicsEngine/*.?pp")
file(GLOB guieng "GuiEngine/*.?pp")
file(GLOB gamelogic "GameLogic/*.?pp")

# Target
add_executable(Test_Unit ${sources} ${baselib} ${gxeng} ${guieng} ${gamelogic})

# Filters
source_group("" FILES ${sources})
source_group("BaseLibrary" FILES ${baselib})
source_group("GraphicsEngine" FILES ${gxeng})
source_group("GuiEngine" FILES ${guieng})
source_group("GameLogic" FILES ${gamelogic})

# Dependencies
target_link_libraries(Test_Unit
	BaseLibrary
	GuiEngine
	GraphicsEngine_LL
	GameLogic
)