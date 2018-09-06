# GRAPHICSENGINE_LL

# Files
file(GLOB interfaces_general "../GraphicsEngine/*.hpp")
file(GLOB interfaces_scene "../GraphicsEngine/Scene/*.hpp")
file(GLOB interfaces_resources "../GraphicsEngine/Resources/*.hpp")

set(common 
	"GraphicsEngine.cpp"
	"GraphicsEngine.hpp"
	"pipeline.json"
	"pipeline_nogi.json"
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
	
	"EntityCollection.hpp"
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

set(memory
	"MemoryManager.cpp"
	"MemoryObject.cpp"
	"ResourceView.cpp"
	
	"MemoryManager.hpp"
	"MemoryObject.hpp"
	"ResourceView.hpp"
)

set(descheaps
	"HostDescHeap.cpp"
	"StackDescHeap.cpp"
	"VolatileViewHeap.cpp"
	
	"HostDescHeap.hpp"
	"StackDescHeap.hpp"
	"VolatileViewHeap.hpp"
)

set (memoryheaps
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

set (backendmisc
	"GraphicsNodeFactory.cpp"
	"ShaderManager.cpp"
	
	"GraphicsNodeFactory.hpp"
	"ShaderManager.hpp"
)

set (backendpipe
	"Pipeline.cpp"
	"PipelineEventDispatcher.cpp"
	"ResourceResidencyQueue.cpp"
	"Scheduler.cpp"
	
	"Pipeline.hpp"
	"PipelineEventDispatcher.hpp"
	"ResourceResidencyQueue.hpp"
	"Scheduler.hpp"
	
	"CommandQueue.hpp"
	"FrameContext.hpp"
	"PipelineEventListener.hpp"
	"SyncPoint.hpp"
)

set (backendpipepool
	"CommandAllocatorPool.cpp"
	"CommandListPool.cpp"
	"ScratchSpacePool.cpp"

	"CommandAllocatorPool.hpp"
	"CommandListPool.hpp"
	"ScratchSpacePool.hpp"
)

set(bridge
	"Binder.cpp"
	"NodeContext.cpp"
	"PipelineTypes.cpp"
	
	"Binder.hpp"
	"NodeContext.hpp"
	"PipelineTypes.hpp"
)

set(bridgelists
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
)

file(GLOB mtlshaders RELATIVE ${CMAKE_CURRENT_LIST_DIR} "Materials/*.hlsl")

set(frontend
	"GraphicsNode.cpp"
	"GraphicsPortConverters.cpp"
	
	"GraphicsNode.hpp"
	"GraphicsPortConverters.hpp"
)

set(nodecommon
	"Nodes/ExampleNode.hpp"
	"Nodes/NodeUtility.hpp"
	"Nodes/NodeUtility.cpp"
)

set(node_dbg
	"Nodes/DebugDrawManager.hpp"
	"Nodes/DebugDraw.cpp"
	"Nodes/DebugDraw.hpp"
)

set(node_draw
	"Nodes/DrawSky.cpp"
	"Nodes/DepthPrepass.cpp"
	"Nodes/RenderOverlay.cpp"
	"Nodes/RenderForwardSimple.cpp"
	
	"Nodes/DrawSky.hpp"
	"Nodes/DepthPrepass.hpp"
	"Nodes/RenderOverlay.hpp"
	"Nodes/RenderForwardSimple.hpp"

	"Nodes/Shaders/RenderOverlay_Overlay.hlsl"
	"Nodes/Shaders/RenderOverlay_Text.hlsl"
	"Nodes/Shaders/RenderForwardSimple.hlsl"
)

set(node_lighting
	"Nodes/ForwardRender.cpp"
	"Nodes/LightCulling.cpp"
	"Nodes/Voxelization.cpp"
	"Nodes/VoxelLighting.cpp"
	"Nodes/VolumetricLighting.cpp"
	
	"Nodes/ForwardRender.hpp"
	"Nodes/LightCulling.hpp"
	"Nodes/Voxelization.hpp"
	"Nodes/VoxelLighting.hpp"
	"Nodes/VolumetricLighting.hpp"
)

set(node_pp_aa
	"Nodes/SMAA.cpp"
	"Nodes/SMAA.hpp"
)
set(node_pp_bloom
	"Nodes/BloomAdd.cpp"
	"Nodes/BloomBlur.cpp"
	"Nodes/BloomDownsample.cpp"
	"Nodes/BrightLumPass.cpp"
	"Nodes/HDRCombine.cpp"
	"Nodes/LensFlare.cpp"
	"Nodes/LuminanceReduction.cpp"
	"Nodes/LuminanceReductionFinal.cpp"
	
	"Nodes/BloomAdd.hpp"
	"Nodes/BloomBlur.hpp"
	"Nodes/BloomDownsample.hpp"
	"Nodes/BrightLumPass.hpp"
	"Nodes/HDRCombine.hpp"
	"Nodes/LensFlare.hpp"
	"Nodes/LuminanceReduction.hpp"
	"Nodes/LuminanceReductionFinal.hpp"
)

set(node_pp_dof
	"Nodes/DOFMain.cpp"
	"Nodes/DOFNeighborMax.cpp"
	"Nodes/DOFPrepare.cpp"
	"Nodes/DOFTileMax.cpp"
	
	"Nodes/DOFMain.hpp"
	"Nodes/DOFNeighborMax.hpp"
	"Nodes/DOFPrepare.hpp"
	"Nodes/DOFTileMax.hpp"
)

set(node_pp_mblur
	"Nodes/MotionBlur.cpp"
	"Nodes/NeighborMax.cpp"
	"Nodes/TileMax.cpp"
	
	"Nodes/MotionBlur.hpp"
	"Nodes/NeighborMax.hpp"
	"Nodes/TileMax.hpp"
)

set(node_pp_ssao
	"Nodes/ScreenSpaceAmbientOcclusion.cpp"
	"Nodes/ScreenSpaceAmbientOcclusion.hpp"
)

set(node_pp_ssr
	"Nodes/ScreenSpaceReflection.cpp"
	"Nodes/ScreenSpaceReflection.hpp"
)

set(node_shadow_csm
	"Nodes/CSM.cpp"
	"Nodes/DepthReduction.cpp"
	"Nodes/DepthReductionFinal.cpp"
	
	"Nodes/CSM.hpp"
	"Nodes/DepthReduction.hpp"
	"Nodes/DepthReductionFinal.hpp"
)
set(node_shadow_pointspot
	"Nodes/ShadowMapGen.cpp"
	"Nodes/ShadowMapGen.hpp"
)
set(node_shadow_screenspace
	"Nodes/ScreenSpaceShadow.cpp"
	"Nodes/ScreenSpaceShadow.hpp"
)
set(node_shadow_filtering
	"Nodes/ShadowFilter.cpp"
	"Nodes/ShadowFilter.hpp"
)

set(node_sys 
	"Nodes/CreateTexture.hpp"
	"Nodes/GetBackBuffer.hpp"
	"Nodes/GetCameraByName.hpp"
	"Nodes/GetCamera2DByName.hpp"
	"Nodes/GetEnvVariable.hpp"
	"Nodes/GetSceneByName.hpp"
	"Nodes/GetTime.hpp"
)

set(node_util 
	"Nodes/TextureProperties.hpp"
	"Nodes/VectorComponents.hpp"
	
	"Nodes/Blend.cpp"
	"Nodes/Blend.hpp"
	
	"Nodes/BlendWithTransform.cpp"
	"Nodes/BlendWithTransform.hpp"
	
	"Nodes/ScreenSpaceTransform.hpp"

	"Nodes/Shaders/Blend.hlsl"
	"Nodes/Shaders/BlendWithTransform.hlsl"
)

set(shaders_dof
	"Nodes/Shaders/DOFMain.hlsl"
	"Nodes/Shaders/DOFNeighborMax.hlsl"
	"Nodes/Shaders/DOFPostfilter.hlsl"
	"Nodes/Shaders/DOFPrepare.hlsl"
	"Nodes/Shaders/DOFTileMax.hlsl"
	"Nodes/Shaders/DOFUpsample.hlsl"
	)

set(shaders_bloom
	"Nodes/Shaders/BloomAdd.hlsl"
	"Nodes/Shaders/BloomBlur.hlsl"
	"Nodes/Shaders/BloomDownsample.hlsl"
	"Nodes/Shaders/BrightLumPass.hlsl"
	"Nodes/Shaders/HDRCombine.hlsl"
	"Nodes/Shaders/LensFlare.hlsl"
	"Nodes/Shaders/LuminanceReduction.hlsl"
	"Nodes/Shaders/LuminanceReductionFinal.hlsl"
	"Nodes/Shaders/LuminanceReductionFinal.hlsl"
	)
	
set(shaders_shadow_csm
	"Nodes/Shaders/CSM.hlsl"
	"Nodes/Shaders/CSMSample.hlsl"
	"Nodes/Shaders/DepthReduction.hlsl"
	"Nodes/Shaders/DepthReductionFinal.hlsl"
	)
	
set(shaders_shadow_point_spot
	"Nodes/Shaders/PointLightShadowMapSample.hlsl"
	"Nodes/Shaders/ShadowGen.hlsl"
	)
	
set(shaders_shadow_filter
	"Nodes/Shaders/ShadowMinfilter.hlsl"
	"Nodes/Shaders/ShadowBlur.hlsl"
	"Nodes/Shaders/ShadowBlurSample.hlsl"
	"Nodes/Shaders/ShadowPenumbra.hlsl"
)
	
set(shaders_debug
	"Nodes/Shaders/DebugDraw.hlsl"
	)
	
set(shaders_lighting
	"Nodes/Shaders/DepthPrepass.hlsl"
	"Nodes/Shaders/DrawSky.hlsl"
	"Nodes/Shaders/LightCulling.hlsl"
	"Nodes/Shaders/LightingUniforms.hlsl"
	"Nodes/Shaders/PbrBrdf.hlsl"
	"Nodes/Shaders/SDFCulling.hlsl"
	"Nodes/Shaders/TiledLighting.hlsl"
	"Nodes/Shaders/VolumetricLighting.hlsl"
	)
	
set(shaders_motion_blur
	"Nodes/Shaders/EncodeDecode.hlsl"
	"Nodes/Shaders/MotionBlur.hlsl"
	"Nodes/Shaders/NeighborMax.hlsl"
	"Nodes/Shaders/TileMax.hlsl"
	)
	
set(shaders_screen_space
	"Nodes/Shaders/ScreenSpaceAmbientOcclusion.hlsl"
	"Nodes/Shaders/SsaoBilateralHorizontalBlur.hlsl"
	"Nodes/Shaders/SsaoBilateralVerticalBlur.hlsl"
	"Nodes/Shaders/ScreenSpaceReflection.hlsl"
	"Nodes/Shaders/SsrBlur.hlsl"
	"Nodes/Shaders/SsrDownsample.hlsl"
	"Nodes/Shaders/ScreenSpaceShadow.hlsl"
	)
	
set(shaders_aa
	"Nodes/Shaders/SMAA.hlsl"
	"Nodes/Shaders/SMAABlendingWeights.hlsl"
	"Nodes/Shaders/SMAAEdgeDetection.hlsl"
	"Nodes/Shaders/SMAANeighborhoodBlending.hlsl"
	)
	
set(shaders_vxgi
	"Nodes/Shaders/VoxelFinalGather.hlsl"
	"Nodes/Shaders/Voxelization.hlsl"
	"Nodes/Shaders/VoxelLightInjectionCSM.hlsl"
	"Nodes/Shaders/VoxelMipmap.hlsl"
	"Nodes/Shaders/VoxelVisualizer.hlsl"
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

	${common} 
	${scene}
	${resources}
	
	${memory}
	${descheaps}
	${memoryheaps}
	${backendmisc}
	${backendpipe}
	${backendpipepool}
	
	${bridge}
	${bridgelists}
	
	${frontend}
	${nodecommon}
	
	${mtlshaders}
	
	${node_dbg}
	${node_draw}
	${node_lighting}
	${node_pp_aa}
	${node_pp_bloom}
	${node_pp_dof}
	${node_pp_mblur}
	${node_pp_ssao}
	${node_pp_ssr}
	${node_shadow_csm}
	${node_shadow_pointspot}
	${node_shadow_screenspace}
	${node_shadow_filtering}
	${node_sys}
	${node_util}
	
	${shaders_dof}
	${shaders_bloom}
	${shaders_shadow_csm}
	${shaders_shadow_point_spot}
	${shaders_shadow_filter}
	${shaders_debug}
	${shaders_lighting}
	${shaders_motion_blur}
	${shaders_screen_space}
	${shaders_aa}
	${shaders_vxgi}

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
source_group("" FILES ${common})
source_group("Interfaces" FILES ${interfaces_general})
source_group("Interfaces\\Scene" FILES ${interfaces_scene})
source_group("Interfaces\\Resources" FILES ${interfaces_resources})

source_group("Scene" FILES ${scene})
source_group("Resources" FILES ${resources})

source_group("Backend\\Memory" FILES ${memory})
source_group("Backend\\Memory\\DescriptorHeaps" FILES ${descheaps})
source_group("Backend\\Memory\\MemoryHeaps" FILES ${memoryheaps})
source_group("Backend\\Misc" FILES ${backendmisc})
source_group("Backend\\Pipeline" FILES ${backendpipe})
source_group("Backend\\Pipeline\\ResourcePools" FILES ${backendpipepool})

source_group("Bridge" FILES ${bridge})
source_group("Bridge\\CommandLists" FILES ${bridgelists})

source_group("Frontend" FILES ${frontend})
source_group("Frontend\\Nodes" FILES ${nodecommon})

source_group("Frontend\\MaterialShaders" FILES ${mtlshaders})

source_group("Frontend\\Nodes\\Debug" FILES ${node_dbg})
source_group("Frontend\\Nodes\\Draw" FILES ${node_draw})
source_group("Frontend\\Nodes\\Lighting" FILES ${node_lighting})
source_group("Frontend\\Nodes\\PostProcess\\AntiAliasing" FILES ${node_pp_aa})
source_group("Frontend\\Nodes\\PostProcess\\HDR" FILES ${node_pp_bloom})
source_group("Frontend\\Nodes\\PostProcess\\DepthOfField" FILES ${node_pp_dof})
source_group("Frontend\\Nodes\\PostProcess\\MotionBlur" FILES ${node_pp_mblur})
source_group("Frontend\\Nodes\\PostProcess\\SSAO" FILES ${node_pp_ssao})
source_group("Frontend\\Nodes\\PostProcess\\SSR" FILES ${node_pp_ssr})
source_group("Frontend\\Nodes\\Shadow\\CascadedShadowMap" FILES ${node_shadow_csm})
source_group("Frontend\\Nodes\\Shadow\\PointSpot" FILES ${node_shadow_pointspot})
source_group("Frontend\\Nodes\\Shadow\\ScreenSpace" FILES ${node_shadow_screenspace})
source_group("Frontend\\Nodes\\Shadow\\Filtering" FILES ${node_shadow_filtering})
source_group("Frontend\\Nodes\\System" FILES ${node_sys})
source_group("Frontend\\Nodes\\Utility" FILES ${node_util})

source_group("Shaders\\DOF" FILES ${shaders_dof})
source_group("Shaders\\Bloom" FILES ${shaders_bloom})
source_group("Shaders\\Shadow\\CSM" FILES ${shaders_shadow_csm})
source_group("Shaders\\Shadow\\PointSpot" FILES ${shaders_shadow_point_spot})
source_group("Shaders\\Shadow\\Filter" FILES ${shaders_shadow_filter})
source_group("Shaders\\Debug" FILES ${shaders_debug})
source_group("Shaders\\Lighting" FILES ${shaders_lighting})
source_group("Shaders\\MotionBlur" FILES ${shaders_motion_blur})
source_group("Shaders\\ScreenSpace" FILES ${shaders_screen_space})
source_group("Shaders\\AA" FILES ${shaders_aa})
source_group("Shaders\\VXGI" FILES ${shaders_vxgi})

source_group("GraphEditors" FILES ${graph_editors})

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
	


