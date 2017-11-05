#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../BasicCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <optional>

namespace inl::gxeng::nodes {


class MotionBlur :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, Texture2D, Texture2D, Texture2D>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	static const char* Info_GetName() { return "MotionBlur"; }
	MotionBlur();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_inputTexBindParam;
	BindParameter m_velocityTexBindParam;
	BindParameter m_neighborMaxTexBindParam;
	BindParameter m_depthTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_motionblur_rtv;

	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;
	bool fsqInited;


protected: // render context
	TextureView2D m_inputTexSrv;
	TextureView2D m_velocityTexSrv;
	TextureView2D m_neighborMaxTexSrv;
	TextureView2D m_depthTexSrv;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

