# GAMELOGIC

# Files comprising the target
set(src_world
	World.cpp
	World.hpp
	Entity.cpp
	Entity.hpp
	ContiguousVector.hpp
	ComponentStore.cpp
	ComponentStore.hpp
	ComponentMatrix.cpp
	ComponentMatrix.hpp
	ComponentMatrixIterator.hpp
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

# Create target
add_library(GameLogic STATIC 
	${src_world}
	${src_factory}
)

# Filters
source_group("World" FILES ${src_world})
source_group("Factory" FILES ${src_factory})

# Dependencies
target_link_libraries(GameLogic
	BaseLibrary
	GraphicsEngine_LL
)
