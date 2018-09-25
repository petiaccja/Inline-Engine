# GUI TEST

# Files
set(sources 
	"main.cpp"
)

# Target
add_executable(Test_GUI ${sources})

# Filters
source_group("" FILES ${sources})

# Dependencies
target_link_libraries(Test_GUI
	BaseLibrary
	AssetLibrary
	GraphicsApi_D3D12
	GraphicsEngine_LL
	GuiEngine
	PhysicsEngine_Bullet
)