# GUIENGINE

# Files comprising the target
set(sources
	Control.hpp
	Control.cpp
	Layout.hpp
	Layout.cpp
	Board.hpp
	Board.cpp
	Sprite.hpp
	Sprite.cpp
	Text.hpp
	Text.cpp
	GraphicsContext.hpp
)

set(sources2
	Control2.hpp
	Control2.cpp
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
	ScrollBar.hpp
	ScrollBar.cpp
	ScrollFrameV.hpp
	ScrollFrameV.cpp

	StandardControl.cpp
	StandardControl.hpp
)

set(layouts
	AbsoluteLayout.hpp
	AbsoluteLayout.cpp
	LinearLayout.hpp
	LinearLayout.cpp
)

set(placeholders
	Placeholders/PlaceholderTextEntity.hpp
	Placeholders/PlaceholderOverlayEntity.hpp)

# Create target
add_library(GuiEngine STATIC 
	${sources} ${sources2} ${controls} ${layouts} ${placeholders}
)

# Filters
source_group("" FILES ${sources})
source_group("Controls" FILES ${controls})
source_group("V2" FILES ${sources2})
source_group("Layouts" FILES ${layouts})
source_group("Placeholders" FILES ${placeholders})


# Dependencies
target_link_libraries(GuiEngine
	BaseLibrary
	GraphicsEngine_LL
)
