#pragma once

#include "GraphicsEngine_LL/Image.hpp"
#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>

namespace inl::gxeng::nodes {


class SMAA : virtual public GraphicsNode,
			 virtual public GraphicsTask,
			 virtual public InputPortConfig<Texture2D, inl::gxeng::Image*, inl::gxeng::Image*, Texture2D>,
			 virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "SMAA"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	SMAA();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	Binder m_binder;
	BindParameter m_inputTexBindParam;
	BindParameter m_areaTexBindParam;
	BindParameter m_searchTexBindParam;
	BindParameter m_blendTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_edgeDetectionShader;
	ShaderProgram m_blendingWeightsShader;
	ShaderProgram m_neighborhoodBlendingShader;
	std::unique_ptr<gxapi::IPipelineState> m_edgeDetectionPSO;
	std::unique_ptr<gxapi::IPipelineState> m_blendingWeightsPSO;
	std::unique_ptr<gxapi::IPipelineState> m_neighborhoodBlendingPSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_edgeDetectionRTV;
	RenderTargetView2D m_blendingWeightsRTV;
	RenderTargetView2D m_neighborhoodBlendingRTV;
	TextureView2D m_edgeDetectionSRV;
	TextureView2D m_blendingWeightsSRV;
	//TextureView2D m_neighborhoodBlendingSRV;

protected: // render context
	TextureView2D m_inputTexSrv;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes