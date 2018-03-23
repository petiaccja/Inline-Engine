# GRAPHICSENGINE_LL

# Files
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
	
	"BasicCamera.hpp"
	"DirectionalLight.hpp"	
	"MeshEntity.hpp"
	"OrthographicCamera.hpp"
	"OverlayEntity.hpp"
	"PerspectiveCamera.hpp"
	"Scene.hpp"
	"TextEntity.hpp"
	"Camera2D.hpp"
	
	"EntityCollection.hpp"
)

set(resources
	"Cubemap.cpp"
	"Font.cpp"
	"Image.cpp"
	"ImageBase.cpp"
	"Material.cpp"
	"Mesh.cpp"
	"MeshBuffer.cpp"
	"VertexCompressor.cpp"
	
	"Cubemap.hpp"
	"Font.hpp"
	"Image.hpp"
	"ImageBase.hpp"
	"Material.hpp"
	"Mesh.hpp"
	"MeshBuffer.hpp"
	"VertexCompressor.hpp"
		
	"Vertex.hpp"
	"Pixel.hpp"
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
	"GraphEditor.cpp"
	"GraphicsNodeFactory.cpp"
	"ShaderManager.cpp"
	
	"GraphEditor.hpp"
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
	"Nodes/Node_DebugDraw.cpp"
	"Nodes/Node_DebugDraw.hpp"
)

set(node_draw
	"Nodes/Node_DrawSky.cpp"
	"Nodes/Node_DepthPrepass.cpp"
	"Nodes/Node_RenderOverlay.cpp"
	
	"Nodes/Node_DrawSky.hpp"
	"Nodes/Node_DepthPrepass.hpp"
	"Nodes/Node_RenderOverlay.hpp"

	"Nodes/Shaders/RenderOverlay_Overlay.hlsl"
	"Nodes/Shaders/RenderOverlay_Text.hlsl"
)

set(node_lighting
	"Nodes/Node_ForwardRender.cpp"
	"Nodes/Node_LightCulling.cpp"
	"Nodes/Node_Voxelization.cpp"
	"Nodes/Node_VolumetricLighting.cpp"
	
	"Nodes/Node_ForwardRender.hpp"
	"Nodes/Node_LightCulling.hpp"
	"Nodes/Node_Voxelization.hpp"
	"Nodes/Node_VolumetricLighting.hpp"
)

set(node_pp_aa
	"Nodes/Node_SMAA.cpp"
	"Nodes/Node_SMAA.hpp"
)
set(node_pp_bloom
	"Nodes/Node_BloomAdd.cpp"
	"Nodes/Node_BloomBlur.cpp"
	"Nodes/Node_BloomDownsample.cpp"
	"Nodes/Node_BrightLumPass.cpp"
	"Nodes/Node_HDRCombine.cpp"
	"Nodes/Node_LensFlare.cpp"
	"Nodes/Node_LuminanceReduction.cpp"
	"Nodes/Node_LuminanceReductionFinal.cpp"
	
	"Nodes/Node_BloomAdd.hpp"
	"Nodes/Node_BloomBlur.hpp"
	"Nodes/Node_BloomDownsample.hpp"
	"Nodes/Node_BrightLumPass.hpp"
	"Nodes/Node_HDRCombine.hpp"
	"Nodes/Node_LensFlare.hpp"
	"Nodes/Node_LuminanceReduction.hpp"
	"Nodes/Node_LuminanceReductionFinal.hpp"
)

set(node_pp_dof
	"Nodes/Node_DOFMain.cpp"
	"Nodes/Node_DOFNeighborMax.cpp"
	"Nodes/Node_DOFPrepare.cpp"
	"Nodes/Node_DOFTileMax.cpp"
	
	"Nodes/Node_DOFMain.hpp"
	"Nodes/Node_DOFNeighborMax.hpp"
	"Nodes/Node_DOFPrepare.hpp"
	"Nodes/Node_DOFTileMax.hpp"
)

set(node_pp_mblur
	"Nodes/Node_MotionBlur.cpp"
	"Nodes/Node_NeighborMax.cpp"
	"Nodes/Node_TileMax.cpp"
	
	"Nodes/Node_MotionBlur.hpp"
	"Nodes/Node_NeighborMax.hpp"
	"Nodes/Node_TileMax.hpp"
)

set(node_pp_ssao
	"Nodes/Node_ScreenSpaceAmbientOcclusion.cpp"
	"Nodes/Node_ScreenSpaceAmbientOcclusion.hpp"
)

set(node_pp_ssr
	"Nodes/Node_ScreenSpaceReflection.cpp"
	"Nodes/Node_ScreenSpaceReflection.hpp"
)

set(node_shadow_csm
	"Nodes/Node_CSM.cpp"
	"Nodes/Node_DepthReduction.cpp"
	"Nodes/Node_DepthReductionFinal.cpp"
	
	"Nodes/Node_CSM.hpp"
	"Nodes/Node_DepthReduction.hpp"
	"Nodes/Node_DepthReductionFinal.hpp"
)
set(node_shadow_pointspot
	"Nodes/Node_ShadowMapGen.cpp"
	"Nodes/Node_ShadowMapGen.hpp"
)
set(node_shadow_screenspace
	"Nodes/Node_ScreenSpaceShadow.cpp"
	"Nodes/Node_ScreenSpaceShadow.hpp"
)

set(node_sys 
	"Nodes/Node_CreateTexture.hpp"
	"Nodes/Node_GetBackBuffer.hpp"
	"Nodes/Node_GetCameraByName.hpp"
	"Nodes/Node_GetCamera2DByName.hpp"
	"Nodes/Node_GetEnvVariable.hpp"
	"Nodes/Node_GetSceneByName.hpp"
	"Nodes/Node_GetTime.hpp"
)

set(node_util 
	"Nodes/Node_TextureProperties.hpp"
	"Nodes/Node_VectorComponents.hpp"
	
	"Nodes/Node_Blend.cpp"
	"Nodes/Node_Blend.hpp"
	
	"Nodes/Node_BlendWithTransform.cpp"
	"Nodes/Node_BlendWithTransform.hpp"
	
	"Nodes/Node_ScreenSpaceTransform.hpp"
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
	"Nodes/Shaders/EncodeVelocity.hlsl"
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
	
set(all_the_files
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
	${node_sys}
	${node_util}
	
	${shaders_dof}
	${shaders_bloom}
	${shaders_shadow_csm}
	${shaders_shadow_point_spot}
	${shaders_debug}
	${shaders_lighting}
	${shaders_motion_blur}
	${shaders_screen_space}
	${shaders_aa}
	${shaders_vxgi}
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
source_group("Frontend\\Nodes\\System" FILES ${node_sys})
source_group("Frontend\\Nodes\\Utility" FILES ${node_util})

source_group("Shaders\\DOF" FILES ${shaders_dof})
source_group("Shaders\\Bloom" FILES ${shaders_bloom})
source_group("Shaders\\Shadow\\CSM" FILES ${shaders_shadow_csm})
source_group("Shaders\\Shadow\\PointSpot" FILES ${shaders_shadow_point_spot})
source_group("Shaders\\Debug" FILES ${shaders_debug})
source_group("Shaders\\Lighting" FILES ${shaders_lighting})
source_group("Shaders\\MotionBlur" FILES ${shaders_motion_blur})
source_group("Shaders\\ScreenSpace" FILES ${shaders_screen_space})
source_group("Shaders\\AA" FILES ${shaders_aa})
source_group("Shaders\\VXGI" FILES ${shaders_vxgi})

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
	


