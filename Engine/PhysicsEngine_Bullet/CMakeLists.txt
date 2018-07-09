# PHYSICSENGINEBULLET

# Files
set(main
	PhysicsEngine.cpp
	PhysicsEngine.hpp
)

set(scene
	RigidBody.cpp
	RigidBody.hpp
	Scene.cpp
	Scene.hpp
)
	
set(resources
	MeshShape.cpp
	MeshShape.hpp
	Shape.cpp
	Shape.hpp
)	

set(utility
	BulletTypes.hpp)
	
file(GLOB interfaces "../PhysicsEngine/*.?pp")

# Target
add_library(PhysicsEngine_Bullet
	${main}
	${interfaces}
	${scene}
	${resources}
	${utility}
)

# Filters
source_group("" FILES ${main})
source_group("Interfaces" FILES ${interfaces})
source_group("Scene" FILES ${scene})
source_group("Resources" FILES ${resources})
source_group("Utility" FILES ${utility})

# Dependencies
target_link_libraries(PhysicsEngine_Bullet
	debug ${EXTERNALS_LIB_DEBUG}/Bullet3Collision-d.lib
	debug ${EXTERNALS_LIB_DEBUG}/Bullet3Common-d.lib
	debug ${EXTERNALS_LIB_DEBUG}/Bullet3Dynamics-d.lib
	debug ${EXTERNALS_LIB_DEBUG}/Bullet3Geometry-d.lib
	debug ${EXTERNALS_LIB_DEBUG}/BulletCollision-d.lib
	debug ${EXTERNALS_LIB_DEBUG}/BulletDynamics-d.lib
	debug ${EXTERNALS_LIB_DEBUG}/BulletSoftBody-d.lib
	debug ${EXTERNALS_LIB_DEBUG}/LinearMath-d.lib
	optimized ${EXTERNALS_LIB_RELEASE}/Bullet3Collision.lib
	optimized ${EXTERNALS_LIB_RELEASE}/Bullet3Common.lib
	optimized ${EXTERNALS_LIB_RELEASE}/Bullet3Dynamics.lib
	optimized ${EXTERNALS_LIB_RELEASE}/Bullet3Geometry.lib
	optimized ${EXTERNALS_LIB_RELEASE}/BulletCollision.lib
	optimized ${EXTERNALS_LIB_RELEASE}/BulletDynamics.lib
	optimized ${EXTERNALS_LIB_RELEASE}/BulletSoftBody.lib
	optimized ${EXTERNALS_LIB_RELEASE}/LinearMath.lib
)