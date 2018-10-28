#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>

namespace inl::gxeng::nodes {


class LightCulling : virtual public GraphicsNode,
					 virtual public GraphicsTask,
					 virtual public InputPortConfig<Texture2D, const BasicCamera*>, // const EntityCollection<PointLight>*>,
					 virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "LightCulling"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	LightCulling();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	Binder m_binder;
	BindParameter m_inputBindParam;
	BindParameter m_outputBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_CSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RWTextureView2D m_lightCullDataUAV;

protected: // render context
	TextureView2D m_depthTexSrv;
	const BasicCamera* m_camera;
	//const EntityCollection<PointLight>* m_lights;

private:
	uint64_t m_width = 0;
	uint64_t m_height = 0;
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes
