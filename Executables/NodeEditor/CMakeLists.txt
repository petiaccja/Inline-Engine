# NODEEDITOR

# Files
set(files
	"main.cpp"
	"SelectPanel.cpp"
	"SelectPanel.hpp"
	"Node.cpp"
	"Node.hpp"
	"GraphPanel.cpp"
	"GraphPanel.hpp"
	"NodeEditor.cpp"
	"NodeEditor.hpp"
	"FileDialog.cpp" 
)

# Target
add_executable(NodeEditor ${files} pipeline.json)

# Filters
source_group("" FILES ${files})

# Dependencies
target_link_libraries(NodeEditor
	BaseLibrary
	GraphicsApi_D3D12
	GraphicsEngine_LL
)

set(INL_NODEEDITOR_PIPELINE_CMAKE ${CMAKE_CURRENT_LIST_DIR}/pipeline.json)
file(TO_NATIVE_PATH ${INL_NODEEDITOR_PIPELINE_CMAKE} INL_NODEEDITOR_PIPELINE)
set(INL_NODEEDITOR_PIPELINE "R\"(${INL_NODEEDITOR_PIPELINE})\"")
add_definitions(-DINL_NODEEDITOR_PIPELINE=${INL_NODEEDITOR_PIPELINE})