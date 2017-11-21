#pragma once


#include "../GraphicsNode.hpp"

#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"

#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <optional>

namespace inl::gxeng::nodes {


/// <summary>
/// Inputs: Blend Destination, Blend source, BlendMode
/// Output: Blend Destination
/// </summary>
class Blend :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, Texture2D, gxapi::RenderTargetBlendState>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	static const char* Info_GetName() { return "Blend"; }
	void Update() override {}
	void Notify(InputPortBase* sender) override {}
	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;

protected:
	gxapi::RenderTargetBlendState m_blendMode;

	std::optional<Binder> m_binder;
	BindParameter m_tex0Param;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	gxapi::eFormat m_renderTargetFormat = gxapi::eFormat::UNKNOWN;

private: // excute context
	RenderTargetView2D m_blendDest;
	TextureView2D m_blendSrc;
};


} // namespace inl::gxeng::nodes
