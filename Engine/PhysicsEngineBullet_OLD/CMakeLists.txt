# PHYSICSENGINEBULLET

# Files
file(GLOB sources "*.?pp")
file(GLOB interfaces "../PhysicsEngine/*.?pp")

# Target
add_library(PhysicsEngineBullet_OLD
	${sources}
	${interfaces}
)

# Filters
source_group("Sources" FILES ${sources})
source_group("Interfaces" FILES ${interfaces})

# Dependencies
target_link_libraries(PhysicsEngineBullet_OLD
	debug ${EXTERNALS_LIB_DEBUG}/Bullet3Collision.lib
	debug ${EXTERNALS_LIB_DEBUG}/Bullet3Common.lib
	debug ${EXTERNALS_LIB_DEBUG}/Bullet3Dynamics.lib
	debug ${EXTERNALS_LIB_DEBUG}/Bullet3Geometry.lib
	debug ${EXTERNALS_LIB_DEBUG}/BulletCollision.lib
	debug ${EXTERNALS_LIB_DEBUG}/BulletDynamics.lib
	debug ${EXTERNALS_LIB_DEBUG}/BulletSoftBody.lib
	debug ${EXTERNALS_LIB_DEBUG}/LinearMath.lib
	optimized ${EXTERNALS_LIB_RELEASE}/Bullet3Collision.lib
	optimized ${EXTERNALS_LIB_RELEASE}/Bullet3Common.lib
	optimized ${EXTERNALS_LIB_RELEASE}/Bullet3Dynamics.lib
	optimized ${EXTERNALS_LIB_RELEASE}/Bullet3Geometry.lib
	optimized ${EXTERNALS_LIB_RELEASE}/BulletCollision.lib
	optimized ${EXTERNALS_LIB_RELEASE}/BulletDynamics.lib
	optimized ${EXTERNALS_LIB_RELEASE}/BulletSoftBody.lib
	optimized ${EXTERNALS_LIB_RELEASE}/LinearMath.lib
)