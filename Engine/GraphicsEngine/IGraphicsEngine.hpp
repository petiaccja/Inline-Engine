#pragma once


#include <BaseLibrary/GraphEditor/IEditorGraph.hpp>
#include <BaseLibrary/Any.hpp>

#include <string>
#include <filesystem>


#undef CreateFont // damn bullshit winapi


namespace inl::gxeng {


class IMesh;
class IImage;
class IFont;
class IMaterial;

class IScene;
class IMeshEntity;
class IOverlayEntity;
class ITextEntity;
class IPerspectiveCamera;
class IOrthographicCamera;
class ICamera2D;


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

	// Update scene

	/// <summary> Redraws the entire screen. </summary>
	/// <param name="elapsed"> Time since last frame in seconds. </param>
	/// <remarks>
	/// Since this call operates on all the entities and resources that compose the scene,
	/// you are not allowed to concurrently modify these objects while Update() is running.
	/// This includes but is not limited to adding new entities to a scene and uploading
	/// data to meshes or images.
	/// </remarks>
	virtual void Update(float elapsed) = 0;

	/// <summary> Rescales the backbuffer. </summary>
	/// <remarks> Causes a pipeline flush, high overhead. </remarks>
	virtual void SetScreenSize(unsigned width, unsigned height) = 0;

	/// <summary> Returns the current backbuffer size in the out parameters. </remarks>
	virtual void GetScreenSize(unsigned& width, unsigned& height) = 0;

	/// <summary> Sets the D3D swap chain to full-screen mode. </summary>
	/// <param name="enable"> True to full screen, false to windowed. </param>
	virtual void SetFullScreen(bool enable) = 0;

	/// <summary> True if the swap chain is currently in full-screen mode. </summary>
	virtual bool GetFullScreen() const = 0;


	// Graph editor interfaces
	virtual IEditorGraph* QueryPipelineEditor() const = 0;
	virtual IEditorGraph* QueryMaterialEditor() const = 0;


	// Resources
	virtual IMesh* CreateMesh() = 0;
	virtual IImage* CreateImage() = 0;
	//virtual IMaterial* CreateMaterial() = 0;
	//virtual IMaterialShaderEquation* CreateMaterialShaderEquation() = 0;
	//virtual IMaterialShaderGraph* CreateMaterialShaderGraph() = 0;
	virtual IFont* CreateFont() = 0;

	// Scene
	virtual IScene* CreateScene(std::string name) = 0;
	//virtual IMeshEntity* CreateMeshEntity() = 0;
	virtual IOverlayEntity* CreateOverlayEntity() = 0;
	virtual ITextEntity* CreateTextEntity() = 0;
	//virtual IPerspectiveCamera* CreatePerspectiveCamera(std::string name) = 0;
	//virtual IOrthographicCamera* CreateOrthographicCamera(std::string name) = 0;
	virtual ICamera2D* CreateCamera2D(std::string name) = 0;


	/// <summary> Creates or sets an environment variable to the given value. </summary>
	/// <returns> True if a new variable was created, false if old was overridden. </returns>
	/// <remarks> Environment variables can be accessed in the graphics pipeline graph by the special
	///		<see cref="nodes::GetEnvVariable"/> node. You can use it to slightly
	///		alter pipeline behavriour from outside. </remarks>
	virtual bool SetEnvVariable(std::string name, Any obj) = 0;

	/// <summary> Returns true if env var with given name exists. </summary>
	virtual bool EnvVariableExists(const std::string& name) = 0;

	/// <summary> Return the env var with given name or throws <see cref="InvalidArgumentException"/>. </summary>
	virtual const Any& GetEnvVariable(const std::string& name) = 0;

	/// <summary> Load the pipeline from the JSON node graph description. </summary>
	/// <remarks> Tears down all the resources associated with the old pipeline, including
	///		textures, render targets, etc., and builds up the new pipeline.
	///		Also incurs a pipeline queue flush. Use it only when settings change,
	///		use env vars to control pipeline behaviour on the fly.
	virtual void LoadPipeline(const std::string& nodes) = 0;

	/// <summary> The engine will look for shader files in these directories. </summary>
	/// <remarks> May be absolute, relative, or whatever paths you OS can handle. </remarks>
	virtual void SetShaderDirectories(const std::vector<std::filesystem::path>& directories) = 0;
};


} // namespace inl::gxeng