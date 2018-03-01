# GUIENGINE

# Files
file(GLOB sources "*.?pp")

# Target
add_library(GuiEngine STATIC ${sources})

# Filters
source_group("" FILES ${sources})

# Dependencies
target_link_libraries(GuiEngine
	BaseLibrary
	
	gdiplus
)