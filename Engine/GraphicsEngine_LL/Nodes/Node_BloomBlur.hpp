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


class BloomBlur :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<Texture2D, mathfu::Vector2f>,
	virtual public exc::OutputPortConfig<Texture2D>
{
public:
	BloomBlur();

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_inputTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_blur_rtv;

	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;
	bool fsqInited;


protected: // render context
	TextureView2D m_inputTexSrv;
	mathfu::Vector2f m_dir;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

