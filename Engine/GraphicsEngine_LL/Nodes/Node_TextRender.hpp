#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../BasicCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"
#include "../Image.hpp"

#include <optional>

namespace inl::gxeng::nodes {

	struct character
	{
		//The left position of the character image in the texture.
		uint16_t x;
		//The top position of the character image in the texture.
		uint16_t y;
		//The width of the character image in the texture.
		uint16_t w;
		//The height of the character image in the texture.
		uint16_t h;
		//How much the current position should be offset when copying the image from the texture to the screen.
		uint16_t xoff;
		//How much the current position should be offset when copying the image from the texture to the screen.
		uint16_t yoff;
		//How much the current position should be advanced after drawing the character.
		uint16_t xadvance;
	};


class TextRender :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, inl::gxeng::Image*, std::vector<char>*>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	static const char* Info_GetName() { return "TextRender"; }
	TextRender();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_fontTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_text_render_rtv;

	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;
	bool fsqInited;

	uint32_t m_lineHeight, m_base, m_texSizeW, m_texSizeH, m_numChars;

	std::vector<character> characters;

protected: // render context

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

