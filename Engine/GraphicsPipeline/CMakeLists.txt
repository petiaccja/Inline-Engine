# GUIENGINE

# Files comprising the target
file(GLOB_RECURSE node_sources "*.?pp")
file(GLOB_RECURSE node_shaders "Shaders/*.hlsl*")
file(GLOB_RECURSE material_shaders "MaterialShaders/*.hlsl*")


# Create target
add_library(GraphicsPipeline STATIC
	${sources} ${node_sources} ${node_shaders} ${material_shaders}
)
set_property(TARGET GraphicsPipeline PROPERTY WHOLE_ARCHIVE true)


# Filters
foreach(src_file IN ITEMS ${node_sources})
    get_filename_component(source_path "${src_file}" DIRECTORY)
    file(RELATIVE_PATH source_path_rel "${CMAKE_CURRENT_LIST_DIR}" "${source_path}")
    string(REPLACE "/" "\\" group_path "${source_path_rel}")
    source_group("${group_path}" FILES "${src_file}")
endforeach()

source_group("Shaders" FILES ${node_shaders})
source_group("MaterialShaders" FILES ${material_shaders})


# Disable HLSL compilation
set_source_files_properties(${node_shaders} ${material_shaders} PROPERTIES VS_TOOL_OVERRIDE "None")


# Dependencies
target_link_libraries(GraphicsPipeline
	BaseLibrary
	GraphicsEngine_LL
)
