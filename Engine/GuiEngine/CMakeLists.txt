# GUIENGINE

# Files comprising the target
set(sources
	Frame.hpp
	Frame.cpp
	Control.hpp
	Control.cpp
	Button.hpp
	Button.cpp
	Label.hpp
	Label.cpp
	TextBox.hpp
	TextBox.cpp
)

# Create target
add_library(GuiEngine STATIC 
	${sources}
)

# Filters
source_group("" FILES ${sources})


# Dependencies
target_link_libraries(GuiEngine
	BaseLibrary
	GraphicsEngine_LL
)
