# GUIENGINE

# Files comprising the target
set(sources
	Control.hpp
	Control.cpp
	Layout.hpp
	Layout.cpp
	ControlTracer.hpp
	ControlTracer.cpp
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

	StandardControl.cpp
	StandardControl.hpp
)

set(layouts
	AbsoluteLayout.hpp
	AbsoluteLayout.cpp
)

set(placeholders
	Placeholders/PlaceholderTextEntity.hpp
	Placeholders/PlaceholderOverlayEntity.hpp)

# Create target
add_library(GuiEngine STATIC 
	${sources} ${controls} ${layouts} ${placeholders}
)

# Filters
source_group("" FILES ${sources})
source_group("Controls" FILES ${controls})
source_group("Layouts" FILES ${layouts})
source_group("Placeholders" FILES ${placeholders})


# Dependencies
target_link_libraries(GuiEngine
	BaseLibrary
	GraphicsEngine_LL
)
