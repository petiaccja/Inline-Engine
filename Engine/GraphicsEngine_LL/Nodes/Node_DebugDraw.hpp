#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../PerspectiveCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <memory>
#include <optional>

namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: render target, scene objects, light cascade MVP transform matrices in a texture
/// Output: render target
/// </summary>
class DebugDraw :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, const BasicCamera*>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	static const char* Info_GetName() { return "DebugDraw"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	DebugDraw();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_LinePSO;
	std::unique_ptr<gxapi::IPipelineState> m_TrianglePSO;

private:
	void BuildVertexBuffers();

private:
	struct MeshData {
		gxeng::VertexBuffer vertexBuffer;
		gxeng::IndexBuffer indexBuffer;
		unsigned size;
		unsigned stride;
	};

	std::vector<std::pair<std::weak_ptr<DebugObject>, MeshData>> m_objects;
	//std::vector<gxeng::VertexBuffer> m_vertexBuffers;
	//std::vector<gxeng::IndexBuffer> m_indexBuffers;
	//std::vector<unsigned> m_sizes;
	//std::vector<unsigned> m_strides;

private: // render context
	RenderTargetView2D m_target;
	const BasicCamera* m_camera;
};


} // namespace inl::gxeng::nodes

