# GAMELOGIC

# Files comprising the target
set(src_base
	GameWorld.cpp
	GameWorld.hpp
	GameEntity.cpp
	GameEntity.hpp
	ContiguousVector.hpp
	EntityStore.cpp
	EntityStore.hpp
	ComponentRange.hpp
	ComponentScheme.hpp
	ComponentScheme.cpp
	Component.hpp
	System.hpp
)

set(src_components
	ComponentLibrary/TransformComponent.hpp)


# Create target
add_library(GameLogic STATIC 
	${src_base}
	${src_components}
)

# Filters
source_group("World" FILES ${src_base})
source_group("ComponentLibrary" FILES ${src_components})

# Dependencies
target_link_libraries(GameLogic
	BaseLibrary
	GraphicsEngine_LL
)
