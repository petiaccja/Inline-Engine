# QC_Simulator

# Files
set(sources 
	"main.cpp"
	"QCWorld.cpp"
	"QCWorld.hpp"
	"RigidBody.cpp"
	"RigidBody.hpp"
	"Rotor.cpp"
	"Rotor.hpp"
	"PIDController.cpp"
	"PIDController.hpp"
)
	
set(extra_sources
	"AreaTex.h"
	"SearchTex.h"
)

# Target
add_executable(QC_Simulator ${sources} ${extra_sources})

# Filters
source_group("" FILES ${sources})
source_group("DoesNotBelongHere" FILES ${extra_sources})

# Dependencies
target_link_libraries(QC_Simulator
	BaseLibrary
	AssetLibrary
	GraphicsApi_D3D12
	GraphicsEngine_LL
)