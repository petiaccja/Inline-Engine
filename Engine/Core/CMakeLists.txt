# CORE

# Files
set(common 
	"Common.hpp")
	
set(global
	"Core.cpp"
	"Core.hpp"
	"InputCore.cpp"
	"InputCore.hpp"
	"TimeCore.hpp"
)

set(script
	"ActorBehavior.cpp"
	"ActorBehavior.hpp"
	"ActorScript.cpp"
	"ActorScript.hpp"
	"SceneScript.cpp"
	"SceneScript.hpp"
)

set(scene
	"Scene.cpp"
	"Scene.hpp"
)

set(part
	"DirectionalLightPart.cpp"
	"MeshPart.cpp"
	"Part.cpp"
	"PerspCameraPart.cpp"
	"RigidBodyPart.cpp"
	"SoftBodyPart.cpp"
	"TransformPart.cpp"
	
	"DirectionalLightPart.hpp"
	"MeshPart.hpp"
	"Part.hpp"
	"PerspCameraPart.hpp"
	"RigidBodyPart.hpp"
	"SoftBodyPart.hpp"
	"TransformPart.hpp"
)

set(actor
	"Actor.cpp"
	"Actor.hpp"
	"Actors.hpp"
)

# Target
add_library(Core STATIC 
	${common}
	${global}
	${script}
	${scene}
	${part}
	${actor}		
)

# Filters
source_group("" FILES ${common})
source_group("" FILES ${global})
source_group("Scripts" FILES ${script})
source_group("Scene" FILES ${scene})
source_group("Parts" FILES ${part})
source_group("Actors" FILES ${actor})

# Dependencies
target_link_libraries(Core
	BaseLibrary
	GraphicsEngine_LL
)