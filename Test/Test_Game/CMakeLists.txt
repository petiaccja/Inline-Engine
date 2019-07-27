# TEST GAME

# Files
set(sources main.cpp UserInterface.hpp UserInterface.cpp)

set(src_ui
	DebugInfoFrame.hpp
	DebugInfoFrame.cpp)

# Target
add_executable(Test_Game ${sources} ${src_ui})

# Filters
source_group("" FILES ${sources})
source_group("UI" FILES ${src_ui})

# Dependencies
target_link_libraries(Test_Game
	AssetLibrary
	BaseLibrary
	GraphicsApi_D3D12
	GraphicsEngine_LL
	GameLogic
	GuiEngine
	GameFoundationLibrary
	GraphicsFoundationLibrary
)