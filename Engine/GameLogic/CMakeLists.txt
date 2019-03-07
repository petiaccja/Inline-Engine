# GAMELOGIC

# Files comprising the target
set(src_base
	GameWorld.cpp
	GameWorld.hpp
	GameEntity.cpp
	GameEntity.hpp
	ComponentVector.cpp
	ComponentVector.hpp
	ComponentStore.cpp
	ComponentStore.hpp
	ComponentRange.hpp
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
