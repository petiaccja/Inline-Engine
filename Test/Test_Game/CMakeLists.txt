# TEST GAME

# Files
set(src_main
	main.cpp
	GameWorld.hpp
	GameWorld.cpp
	GameInterface.hpp
	GameInterface.cpp
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
	WindowLayout.hpp
	WindowLayout.cpp)

set(src_systems
	WindowEventSystem.hpp
	WindowEventSystem.cpp
	UserInterfaceSystem.hpp
	UserInterfaceSystem.cpp)
	
# Target
add_executable(Test_Game ${src_main} ${src_ui} ${src_collections} ${src_systems})

# Filters
source_group("" FILES ${src_main})
source_group("UI" FILES ${src_ui})
source_group("FacilityCollections" FILES ${src_collections})
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