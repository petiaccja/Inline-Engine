# ENGINE AGGREGATE FILE

add_subdirectory(BaseLibrary)
if (TARGET_PLATFORM_WINDOWS)
    add_subdirectory(GraphicsApi_D3D12)
endif()
add_subdirectory(GraphicsEngine_LL)
add_subdirectory(GraphicsPipeline)
add_subdirectory(AssetLibrary)
add_subdirectory(GuiEngine)
#add_subdirectory(Core)
#add_subdirectory(PhysicsEngineBullet_OLD)
add_subdirectory(PhysicsEngine_Bullet)