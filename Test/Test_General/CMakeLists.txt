# TESTGENERAL

# Files
set(sources main.cpp Test.hpp)
file(GLOB cases "Test_*.cpp")

# Target
add_executable(Test_General ${sources} ${cases})

# Filters
source_group("" FILES ${sources})
source_group("Cases" FILES ${cases})

# Dependencies
target_link_libraries(Test_General
	BaseLibrary
	GraphicsEngine_LL
)