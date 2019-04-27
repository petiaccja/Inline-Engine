# GAMELOGIC

# Files comprising the target
set(src_base
	World.cpp
	World.hpp
	Entity.cpp
	Entity.hpp
	ContiguousVector.hpp
	ComponentStore.cpp
	ComponentStore.hpp
	ComponentRange.hpp
	ComponentScheme.hpp
	ComponentScheme.cpp
	System.hpp
)

# Create target
add_library(GameLogic STATIC 
	${src_base}
)

# Filters
source_group("World" FILES ${src_base})

# Dependencies
target_link_libraries(GameLogic
	BaseLibrary
	GraphicsEngine_LL
)
