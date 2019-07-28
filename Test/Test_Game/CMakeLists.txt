# TEST GAME

# Files
set(src_main main.cpp UserInterface.hpp UserInterface.cpp)

set(src_collections
	ModuleCollection.hpp
	ModuleCollection.cpp)

set(src_ui
	DebugInfoFrame.hpp
	DebugInfoFrame.cpp)

# Target
add_executable(Test_Game ${src_main} ${src_ui} ${src_collections})

# Filters
source_group("" FILES ${src_main})
source_group("UI" FILES ${src_ui})
source_group("FacilityCollections" FILES ${src_collections})

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