# PHYSICS TEST

# Files
set(sources 
	"main.cpp"
)

# Target
add_executable(Test_Physics ${sources})

# Filters
source_group("" FILES ${sources})

# Dependencies
target_link_libraries(Test_Physics
	BaseLibrary
	AssetLibrary
	GraphicsApi_D3D12
	GraphicsEngine_LL
	PhysicsEngine_Bullet
)