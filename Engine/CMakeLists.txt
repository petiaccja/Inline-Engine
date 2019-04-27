# ENGINE AGGREGATE FILE

add_subdirectory(BaseLibrary)
if (TARGET_PLATFORM_WINDOWS)
    add_subdirectory(GraphicsApi_D3D12)
endif()
add_subdirectory(GraphicsEngine_LL)
add_subdirectory(GraphicsFoundationLibrary)
add_subdirectory(AssetLibrary)
add_subdirectory(GuiEngine)
add_subdirectory(GameLogic)
add_subdirectory(GameFoundationLibrary)