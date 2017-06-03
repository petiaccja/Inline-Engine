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


class LuminanceReductionFinal :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<Texture2D>,
	virtual public exc::OutputPortConfig<Texture2D>
{
public:
	LuminanceReductionFinal();

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_reductionBindParam;
	BindParameter m_outputBindParam0;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_CSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RWTextureView2D m_avg_lum_uav;

protected: // render context
	TextureView2D m_reductionTexSrv;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

