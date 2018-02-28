# TESTUNIT

# Files
set(sources main.cpp)
file(GLOB baselib "BaseLibrary/*.?pp")

# Target
add_executable(Test_Unit ${sources} ${baselib})

# Filters
source_group("" FILES ${sources})
source_group("BaseLibrary" FILES ${baselib})

# Dependencies
target_link_libraries(Test_Unit
	BaseLibrary
)