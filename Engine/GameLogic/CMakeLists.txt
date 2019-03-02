# GAMELOGIC

# Files comprising the target
set(src_world
	GameWorld.cpp
	GameWorld.hpp
)


# Create target
add_library(GameLogic STATIC 
	${src_world}
)

# Filters
source_group("World" FILES ${src_world})

# Dependencies
target_link_libraries(GameLogic
	BaseLibrary
	GraphicsEngine_LL
)
