# GRAPHICSAPI_D3D12

# Files
file(GLOB sources "*.?pp")
file (GLOB interfaces "../GraphicsApi_LL/*.?pp")

# Target
add_library(GraphicsApi_D3D12 STATIC ${sources} ${interfaces})

# Filters
source_group("Implementation" FILES ${sources})
source_group("Interfaces" FILES ${interfaces})

# Dependencies
target_link_libraries(GraphicsApi_D3D12
	BaseLibrary
	
	d3d12
	dxgi
	d3dcompiler.lib
	debug ${EXTERNALS_LIB_DEBUG}/WinPixEventRuntime.lib
	optimized ${EXTERNALS_LIB_RELEASE}/WinPixEventRuntime.lib
)