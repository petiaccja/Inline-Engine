# NODEEDITOR V2

# Files
set(files
	"main.cpp"
	"NodeEditor.cpp"
	"NodeEditor.hpp"
	"NodePanel.cpp"
	"NodePanel.hpp"
	"NodePropertiesPanel.cpp"
	"NodePropertiesPanel.hpp"
	"PortPropertiesPanel.cpp"
	"PortPropertiesPanel.hpp"
	"NodeSelectPanel.cpp"
	"NodeSelectPanel.hpp"
	"NodeControl.cpp"
	"NodeControl.hpp"
	"PortControl.cpp"
	"PortControl.hpp"
	"ArrowControl.cpp"
	"ArrowControl.hpp"
	"GraphController.cpp"
	"GraphController.hpp"
)

# Target
add_executable(NodeEditor_v2 ${files})

# Filters
source_group("" FILES ${files})

# Dependencies
target_link_libraries(NodeEditor_v2
	BaseLibrary
	GraphicsApi_D3D12
	GraphicsEngine_LL
	GraphicsPipeline
	GuiEngine
)