# GAMELOGIC

# Files comprising the target
set(src_world
	Scene.cpp
	Scene.hpp
	Simulation.cpp
	Simulation.hpp
	Entity.cpp
	Entity.hpp
	ContiguousVector.hpp
	ComponentMatrix.cpp
	ComponentMatrix.hpp
	ComponentVector.hpp
	ComponentRange.hpp
	ComponentScheme.hpp
	ComponentScheme.cpp
	System.hpp
	)

set(src_factory
	ComponentFactory.hpp
	ComponentFactory.cpp
	ComponentClassFactory.hpp
	AutoRegisterComponent.hpp
	VariantArchive.hpp
	Archive.hpp
	)
	
set(src_level
	BasicLevel.hpp
	Level.hpp
	Level.cpp
	LevelArchive.hpp
	LevelArchive.cpp
	)

# Create target
add_library(GameLogic STATIC 
	${src_world}
	${src_factory}
	${src_level}
)

# Filters
source_group("World" FILES ${src_world})
source_group("Factory" FILES ${src_factory})
source_group("Level" FILES ${src_level})

# Dependencies
target_link_libraries(GameLogic
	BaseLibrary
	GraphicsEngine_LL
)
