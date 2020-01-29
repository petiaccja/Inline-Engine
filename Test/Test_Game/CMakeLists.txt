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
	TestLevelSystem.cpp	
	CameraMoveSystem.hpp
	CameraMoveSystem.cpp
	)


set(src_misc
	ActionHeap.hpp
	)
	
# Target
add_executable(Test_Game ${src_main} ${src_ui} ${src_systems} ${src_misc})

# Filters
source_group("" FILES ${src_main})
source_group("UI" FILES ${src_ui})
source_group("Systems" FILES ${src_systems})
source_group("Misc" FILES ${src_misc})

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