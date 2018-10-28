# GRAPHICSENGINE_LL

# Files
file(GLOB interfaces_general "../GraphicsEngine/*.hpp")
file(GLOB interfaces_scene "../GraphicsEngine/Scene/*.hpp")
file(GLOB interfaces_resources "../GraphicsEngine/Resources/*.hpp")

set(main 
	"GraphicsEngine.cpp"
	"GraphicsEngine.hpp"
)

set(scene
	"BasicCamera.cpp"
	"DirectionalLight.cpp"	
	"MeshEntity.cpp"
	"OrthographicCamera.cpp"
	"OverlayEntity.cpp"
	"PerspectiveCamera.cpp"
	"Scene.cpp"
	"TextEntity.cpp"
	"Camera2D.cpp"
	"AnimationState.cpp"
	
	"BasicCamera.hpp"
	"DirectionalLight.hpp"	
	"MeshEntity.hpp"
	"OrthographicCamera.hpp"
	"OverlayEntity.hpp"
	"PerspectiveCamera.hpp"
	"Scene.hpp"
	"TextEntity.hpp"
	"Camera2D.hpp"
	"AnimationState.hpp"
)

set(resources
	"Cubemap.cpp"
	"Font.cpp"
	"Image.cpp"
	"ImageBase.cpp"
	"Material.cpp"
	"MaterialShader.cpp"
	"Mesh.cpp"
	"MeshBuffer.cpp"
	"VertexCompressor.cpp"
	
	"Cubemap.hpp"
	"Font.hpp"
	"Image.hpp"
	"ImageBase.hpp"
	"Material.hpp"
	"MaterialShader.hpp"
	"Mesh.hpp"
	"MeshBuffer.hpp"
	"VertexCompressor.hpp"
)

set(memory_resource
	"MemoryManager.cpp"
	"MemoryObject.cpp"
	"ResourceView.cpp"
	
	"MemoryManager.hpp"
	"MemoryObject.hpp"
	"ResourceView.hpp"
)

set(memory_descheaps
	"HostDescHeap.cpp"
	"StackDescHeap.cpp"
	"VolatileViewHeap.cpp"
	
	"HostDescHeap.hpp"
	"StackDescHeap.hpp"
	"VolatileViewHeap.hpp"
)

set (memory_resheaps
	"BackBufferManager.cpp"
	"ConstBufferHeap.cpp"
	"CriticalBufferHeap.cpp"
	"UploadManager.cpp"
	
	"BackBufferManager.hpp"
	"ConstBufferHeap.hpp"
	"CriticalBufferHeap.hpp"
	"UploadManager.hpp"

	"BufferHeap.hpp"
)

set (pipeline_misc
	"GraphicsNodeFactory.cpp"
	"ShaderManager.cpp"
	
	"GraphicsNodeFactory.hpp"
	"ShaderManager.hpp"
)

set (pipeline_scheduling
	"Pipeline.cpp"
	"PipelineEventDispatcher.cpp"
	"ResourceResidencyQueue.cpp"
	"Scheduler.cpp"
	"SchedulerCPU.cpp"
	"SchedulerGPU.cpp"
	
	"Pipeline.hpp"
	"PipelineEventDispatcher.hpp"
	"ResourceResidencyQueue.hpp"
	"Scheduler.hpp"
	"SchedulerCPU.hpp"
	"SchedulerGPU.hpp"
	
	"CommandQueue.hpp"
	"FrameContext.hpp"
	"PipelineEventListener.hpp"
	"SyncPoint.hpp"
)

set(pipeline_lists
	"BasicCommandList.cpp"
	"CopyCommandList.cpp"
	"ComputeCommandList.cpp"
	"GraphicsCommandList.cpp"
	
	"BasicCommandList.hpp"
	"CopyCommandList.hpp"
	"ComputeCommandList.hpp"
	"GraphicsCommandList.hpp"
	
	"BindingManager.hpp"
	"RootTableManager.hpp"
	
	"Binder.cpp"
	"Binder.hpp"
)


set (pipeline_pools
	"CommandAllocatorPool.cpp"
	"CommandListPool.cpp"
	"ScratchSpacePool.cpp"

	"CommandAllocatorPool.hpp"
	"CommandListPool.hpp"
	"ScratchSpacePool.hpp"
)

set(nodeapi
	"NodeContext.cpp"	
	"NodeContext.hpp"

	"Nodes/NodeUtility.hpp"
	"Nodes/NodeUtility.cpp"
	
	"AutoRegisterNode.hpp"

	"GraphicsNode.cpp"
	"GraphicsPortConverters.cpp"
	
	"GraphicsNode.hpp"
	"GraphicsPortConverters.hpp"

	"Nodes/ExampleNode.hpp"
)


set(sysnodes
	"Nodes/System/CreateTexture.hpp"
	"Nodes/System/GetBackBuffer.hpp"
	"Nodes/System/GetCamera2DByName.hpp"
	"Nodes/System/GetCameraByName.hpp"
	"Nodes/System/GetEnvVariable.hpp"
	"Nodes/System/GetSceneByName.hpp"
	"Nodes/System/GetTime.hpp"
	"Nodes/System/RegisterSystemNodes.cpp"
	"Nodes/System/RegisterSystemNodes.hpp"
)

set(graph_editors
	"GraphEditor/PipelineEditorGraph.cpp"
	"GraphEditor/PipelineEditorGraph.hpp"
	"GraphEditor/PipelineEditorNode.cpp"
	"GraphEditor/PipelineEditorNode.hpp"
	"GraphEditor/MaterialEditorGraph.cpp"
	"GraphEditor/MaterialEditorGraph.hpp"
	"GraphEditor/MaterialEditorNode.cpp"
	"GraphEditor/MaterialEditorNode.hpp"
)
	
set(all_the_files
	${interfaces_general}
	${interfaces_scene}
	${interfaces_resources}

	${main} 
	${scene}
	${resources}
	
	${memory_descheaps}
	${memory_resource}
	${memory_resheaps}

	${pipeline_misc}
	${pipeline_lists}
	${pipeline_scheduling}
	${pipeline_pools}
	
	${nodeapi}
	${sysnodes}

	${graph_editors}
)
	
file(GLOB_RECURSE all_the_rest RELATIVE ${CMAKE_CURRENT_LIST_DIR} "*.?pp" "*.hlsl")
set(all_the_rest_hlsl ${all_the_rest})
list(REMOVE_ITEM all_the_rest ${all_the_files})
list(FILTER all_the_rest_hlsl INCLUDE REGEX ".*\.hlsl")

# Target
add_library(GraphicsEngine_LL STATIC 
	${all_the_files}
	${all_the_rest}
)
set_source_files_properties(${all_the_rest_hlsl} PROPERTIES VS_TOOL_OVERRIDE "None")

# Filters
source_group("Interfaces" FILES ${interfaces_general})
source_group("Interfaces\\Scene" FILES ${interfaces_scene})
source_group("Interfaces\\Resources" FILES ${interfaces_resources})

source_group("" FILES ${main})
source_group("Scene" FILES ${scene})
source_group("Resources" FILES ${resources})

source_group("Memory\\DescriptorHeaps" FILES ${memory_descheaps})
source_group("Memory\\Resources" FILES ${memory_resource})
source_group("Memory\\ResourceHeaps" FILES ${memory_resheaps})

source_group("Pipeline\\Misc" FILES ${pipeline_misc})
source_group("Pipeline\\CommandLists" FILES ${pipeline_lists})
source_group("Pipeline\\Scheduling" FILES ${pipeline_scheduling})
source_group("Pipeline\\Pools" FILES ${pipeline_pools})

source_group("NodeAPI" FILES ${nodeapi})
source_group("SystemNodes" FILES ${sysnodes})

source_group("NodeEditors" FILES ${graph_editors})

source_group("!!!PleaseGroupThese!!!" FILES ${all_the_rest})


# Dependencies
target_link_libraries(GraphicsEngine_LL
	BaseLibrary
	GraphicsApi_D3D12
	
	debug ${EXTERNALS_LIB_DEBUG}/freetype.lib
	optimized ${EXTERNALS_LIB_RELEASE}/freetype.lib
	debug ${EXTERNALS_LIB_DEBUG}/lemon.lib
	optimized ${EXTERNALS_LIB_RELEASE}/lemon.lib
)
	


