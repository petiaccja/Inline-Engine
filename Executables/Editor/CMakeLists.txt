# EDITOR

# Files
set(sources
	"Editor.cpp"
	"Editor.hpp"
	"GeneralCamera.cpp"
	"GeneralCamera.hpp"
	"main.cpp"
)

set(wrongplace
	"AreaTex.h"
	"SearchTex.h"
)

set(garbage
	"PlayerScript.cpp"
	"PlayerScript.hpp"
	"TestLevelScript.cpp"
	"TestLevelScript.hpp"
)

# Target
add_library(Editor STATIC ${sources} ${wrongplace} ${garbage})

# Filters
source_group("" FILES ${sources})
source_group("!!!DoesNotBelongHere!!!" FILES ${wrongplace})
source_group("!!!WhyIsThisHere!!!" FILES ${garbage})

# Dependencies
target_link_libraries(Editor
	BaseLibrary
	GraphicsEngine_LL
)