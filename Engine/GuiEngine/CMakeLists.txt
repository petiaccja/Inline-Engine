# GUIENGINE

# Files comprising the target
set(sources
	Control.hpp
	Control.cpp
	Layout.hpp
	Layout.cpp
)

set(controls 
	Frame.hpp
	Frame.cpp
	Button.hpp
	Button.cpp
	Label.hpp
	Label.cpp
	TextBox.hpp
	TextBox.cpp
)

set(layouts
	AbsoluteLayout.hpp
	AbsoluteLayout.cpp
)

# Create target
add_library(GuiEngine STATIC 
	${sources} ${controls} ${layouts}
)

# Filters
source_group("" FILES ${sources})
source_group("Controls" FILES ${controls})
source_group("Layouts" FILES ${layouts})


# Dependencies
target_link_libraries(GuiEngine
	BaseLibrary
	GraphicsEngine_LL
)
