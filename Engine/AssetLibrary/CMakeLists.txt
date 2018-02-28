# ASSETLIBRARY

# Files
set(files
	"Image.cpp"
	"Image.hpp"
	"Model.cpp"
	"Model.hpp"
)

# Target
add_library(AssetLibrary STATIC ${files})


# Filters
source_group("" FILES ${files})

# Dependencies
target_link_libraries(AssetLibrary
	BaseLibrary
	GraphicsEngine_LL
	
	debug ${EXTERNALS_LIB_DEBUG}/FreeImage.lib
	optimized ${EXTERNALS_LIB_RELEASE}/FreeImage.lib
	debug ${EXTERNALS_LIB_DEBUG}/FreeImagePlus.lib
	optimized ${EXTERNALS_LIB_RELEASE}/FreeImagePlus.lib
	debug ${EXTERNALS_LIB_DEBUG}/assimp-vc140-mt.lib
	optimized ${EXTERNALS_LIB_RELEASE}/assimp-vc140-mt.lib
)
	