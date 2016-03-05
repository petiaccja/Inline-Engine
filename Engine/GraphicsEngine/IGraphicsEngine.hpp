#pragma once

#include "Definitions.hpp"

#include <string>


namespace inl {
namespace gxeng {

class IMesh;
class IMaterial;
class ITexture2D;

class IScene;
class ILight;
class IMeshEntity;
class ITerrainEntity;
class ICamera;

/// <summary>
/// <para>The graphics engine handles the rendering of a 3D scene.</para>
///	<para>A scene consists of many entities, lights and other things.
///		These are created by the graphics engine, and are grouped together
///		into scene objects. An engine may create any number of scenes and
///		entities. </para>
/// <para>Entities may have properties such as geometry, material or texture.
///		These properties are called resources, and are also created by the 
///		graphics engine.</para>
/// <para>A graphics engine requires a low-level rendering backend (i.e. dx12),
///		a target window and a rendering pipeline definition to function.
///		The backend and the window is provided by external services, while
///		the rendering pipeline definition is assembled by building blocks
///		provided by the graphics engine itself.</para>
/// </summary>
class IGraphicsEngine {
public:
	virtual ~IGraphicsEngine() = default;

	// Where to render.
	virtual void SetResolution(int width, int height) = 0;
	virtual void SetViewport(int top, int bottom, int left, int right) = 0;
	virtual void SetTargetWindow(NativeWindowHandle handle) = 0;

	// Create resources.
	virtual IMesh* CreateMesh() = 0;
	virtual IMaterial* CreateMaterial() = 0;
	virtual ITexture2D* CreateTexture2D() = 0;

	// Create scene.
	virtual IScene* CreateScene() = 0;
	virtual ILight* CreateLight() = 0;
	virtual IMeshEntity* CreateMeshEntity() = 0;
	virtual ITerrainEntity* CreateTerrainEntity() = 0;
	virtual ICamera* CreateCamera() = 0;

	// Rendering.
	virtual void RenderWorld(double elapsed) = 0;
	virtual void SetPipeline(std::string definition) = 0;
};


} // namespace gxeng
} // namespace inl