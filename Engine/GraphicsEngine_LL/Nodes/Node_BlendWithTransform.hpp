#pragma once

#include "../GraphicsNode.hpp"

#include "../PipelineTypes.hpp"

#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <mathfu/mathfu_exc.hpp>

#include <optional>

namespace inl::gxeng::nodes {


/// <summary>
/// Inputs: Blend Destination, Blend source, BlendMode, Transform to be applied to the full screen quad in NDC
/// Output: Blend Destination
/// </summary>
class BlendWithTransform :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<Texture2D, Texture2D, gxapi::RenderTargetBlendState, mathfu::Matrix4x4f>,
	virtual public exc::OutputPortConfig<Texture2D>
{
public:
	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
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
	BindParameter m_transformParam;
	BindParameter m_tex0Param;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	gxapi::eFormat m_renderTargetFormat = gxapi::eFormat::UNKNOWN;

private: // excute context
	RenderTargetView2D m_blendDest;
	TextureView2D m_blendSrc;

	mathfu::Matrix4x4f m_transfrom;
};


} // namespace inl::gxeng::nodes
