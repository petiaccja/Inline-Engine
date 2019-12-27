# TEST GAME

# Files
set(src_main
	main.cpp
	Game.hpp
	Game.cpp
	EngineCollection.hpp
	EngineCollection.cpp)

set(src_ui
	UserInterfaceCompositor.hpp
	UserInterfaceCompositor.cpp
	DebugInfoFrame.hpp
	DebugInfoFrame.cpp
	MainMenuFrame.hpp
	MainMenuFrame.cpp
	GameSceneFrame.hpp
	GameSceneFrame.cpp
	WindowLayout.hpp
	WindowLayout.cpp)

set(src_systems
	UserInterfaceSystem.hpp
	UserInterfaceSystem.cpp
	TestLevelSystem.hpp
	TestLevelSystem.cpp)

	
# Target
add_executable(Test_Game ${src_main} ${src_ui} ${src_systems})

# Filters
source_group("" FILES ${src_main})
source_group("UI" FILES ${src_ui})
source_group("Systems" FILES ${src_systems})

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