# GAME FOUNDATION LIBRARY

# Files comprising the target
file(GLOB_RECURSE src_all "*.?pp")

# Create target
add_library(GameFoundationLibrary STATIC 
	${src_all}
)

# Filters
foreach(src_file IN ITEMS ${src_all})
    get_filename_component(source_path "${src_file}" DIRECTORY)
    file(RELATIVE_PATH source_path_rel "${CMAKE_CURRENT_LIST_DIR}" "${source_path}")
    string(REPLACE "/" "\\" group_path "${source_path_rel}")
    source_group("${group_path}" FILES "${src_file}")
endforeach()

# Dependencies
target_link_libraries(GameFoundationLibrary
	BaseLibrary
	GraphicsEngine_LL
	GameLogic
)
