# TEST GAME

# Files
set(src_main
	main.cpp
	Game.hpp
	Game.cpp
	Interface.hpp
	Interface.cpp
	UserInterfaceCompositor.hpp
	UserInterfaceCompositor.cpp)

set(src_collections
	ModuleCollection.hpp
	ModuleCollection.cpp
	AssetCacheCollection.hpp
	AssetCacheCollection.cpp)

set(src_ui
	DebugInfoFrame.hpp
	DebugInfoFrame.cpp
	MainMenuFrame.hpp
	MainMenuFrame.cpp
	GameSceneFrame.hpp
	GameSceneFrame.cpp
	WindowLayout.hpp
	WindowLayout.cpp)

set(src_systems
	WindowEventSystem.hpp
	WindowEventSystem.cpp
	UserInterfaceSystem.hpp
	UserInterfaceSystem.cpp)

set(src_level
	ILevel.hpp
	TestLevel.hpp
	TestLevel.cpp)
	
# Target
add_executable(Test_Game ${src_main} ${src_ui} ${src_collections} ${src_systems} ${src_level})

# Filters
source_group("" FILES ${src_main})
source_group("UI" FILES ${src_ui})
source_group("FacilityCollections" FILES ${src_collections})
source_group("Systems" FILES ${src_systems})
source_group("Level" FILES ${src_level})

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