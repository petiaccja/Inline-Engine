# ASSETLIBRARY

# Files
set(src_primitives
	"Image.cpp"
	"Image.hpp"
	"Model.cpp"
	"Model.hpp"
)

set(main
	"AssetStore.cpp"
	"AssetStore.hpp"	
)

set(src_assetcache
	"AssetCache.hpp"
	"GraphicsMeshCache.cpp"
	"GraphicsMeshCache.hpp"
	"ImageCache.cpp"
	"ImageCache.hpp"
	"MaterialShaderCache.cpp"
	"MaterialShaderCache.hpp"
	"MaterialCache.cpp"
	"MaterialCache.hpp"
)

# Target
add_library(AssetLibrary STATIC
	${src_primitives}
	${main}
	${src_assetcache}
)

# Filters
source_group("Primitives" FILES ${src_primitives})
source_group("Deprecated" FILES ${main})
source_group("Caches" FILES ${src_assetcache})

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
	