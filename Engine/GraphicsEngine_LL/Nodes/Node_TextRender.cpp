#include "Node_TextRender.hpp"

#include "NodeUtility.hpp"

#include "../MeshEntity.hpp"
#include "../DirectionalLight.hpp"
#include "../PerspectiveCamera.hpp"
#include "../GraphicsCommandList.hpp"
#include "../EntityCollection.hpp"

#include "DebugDrawManager.hpp"

#include <array>

namespace inl::gxeng::nodes {

struct Uniforms
{
	Mat44_Packed trans;
	uint32_t x, y, w, h;
	Vec4_Packed color;
};


TextRender::TextRender() {
	this->GetInput<0>().Set({});
	this->GetInput<1>().Set({});
}


void TextRender::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void TextRender::Reset() {
	GetInput<0>().Clear();
	GetInput<1>().Clear();
}


void TextRender::Setup(SetupContext& context) {
	Texture2D& renderTarget = this->GetInput<0>().Get();
	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_text_render_rtv = context.CreateRtv(renderTarget, renderTarget.GetFormat(), rtvDesc);

	auto colorImage = this->GetInput<1>().Get();
	if (colorImage == nullptr) {
		throw InvalidArgumentException("Adjál rendes texturát!");
		if (!colorImage->GetSrv()) {
			throw InvalidArgumentException("Given texture was empty.");
		}
	}

	auto fontBinary = this->GetInput<2>().Get();
	if (fontBinary == nullptr) {
		throw InvalidArgumentException("Adjál rendes font filet!");
		if (fontBinary->empty()) {
			throw InvalidArgumentException("Given font binary was empty.");
		}
	}

	if (!m_binder.has_value()) {
		BindParameterDesc uniformsBindParamDesc;
		m_uniformsBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
		uniformsBindParamDesc.parameter = m_uniformsBindParam;
		uniformsBindParamDesc.constantSize = sizeof(Uniforms);
		uniformsBindParamDesc.relativeAccessFrequency = 0;
		uniformsBindParamDesc.relativeChangeFrequency = 0;
		uniformsBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc sampBindParamDesc;
		sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
		sampBindParamDesc.constantSize = 0;
		sampBindParamDesc.relativeAccessFrequency = 0;
		sampBindParamDesc.relativeChangeFrequency = 0;
		sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc fontBindParamDesc;
		m_fontTexBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		fontBindParamDesc.parameter = m_fontTexBindParam;
		fontBindParamDesc.constantSize = 0;
		fontBindParamDesc.relativeAccessFrequency = 0;
		fontBindParamDesc.relativeChangeFrequency = 0;
		fontBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, fontBindParamDesc },{ samplerDesc });
	}

	if (!m_fsq.HasObject()) {
		std::vector<float> vertices = {
			-1, -1, 0,  0, +1,
			+1, -1, 0, +1, +1,
			+1, +1, 0, +1,  0,
			-1, +1, 0,  0,  0
		};
		std::vector<uint16_t> indices = {
			0, 1, 2,
			0, 2, 3
		};
		m_fsq = context.CreateVertexBuffer(vertices.data(), sizeof(float)*vertices.size());
		m_fsq.SetName("Text render quad vertex buffer");
		m_fsqIndices = context.CreateIndexBuffer(indices.data(), sizeof(uint16_t)*indices.size(), indices.size());
		m_fsqIndices.SetName("Text render quad index buffer");
	}

	if (!m_PSO) {
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_shader = context.CreateShader("TextRender", shaderParts, "");

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
			gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
		};

		gxapi::GraphicsPipelineStateDesc psoDesc;
		psoDesc.inputLayout.elements = inputElementDesc.data();
		psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
		psoDesc.rootSignature = m_binder->GetRootSignature();
		psoDesc.vs = m_shader.vs;
		psoDesc.ps = m_shader.ps;
		psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
		psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
		psoDesc.blending.alphaToCoverage = false;
		psoDesc.blending.independentBlending = false;

		gxapi::RenderTargetBlendState blending;
		blending.enableBlending = true;
		blending.alphaOperation = gxapi::eBlendOperation::ADD;
		blending.shaderAlphaFactor = gxapi::eBlendOperand::SHADER_ALPHA;
		blending.targetAlphaFactor = gxapi::eBlendOperand::INV_SHADER_ALPHA;
		blending.colorOperation = gxapi::eBlendOperation::ADD;
		blending.shaderColorFactor = gxapi::eBlendOperand::SHADER_ALPHA;
		blending.targetColorFactor = gxapi::eBlendOperand::INV_SHADER_ALPHA;
		blending.enableLogicOp = false;
		blending.mask = gxapi::eColorMask::ALL;

		psoDesc.blending.singleTarget = blending;

		psoDesc.depthStencilState.enableDepthTest = false;
		psoDesc.depthStencilState.enableDepthStencilWrite = false;
		psoDesc.depthStencilState.enableStencilTest = false;
		psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

		psoDesc.numRenderTargets = 1;
		psoDesc.renderTargetFormats[0] = m_text_render_rtv.GetResource().GetFormat();

		m_PSO.reset(context.CreatePSO(psoDesc));
	}

	this->GetOutput<0>().Set(m_text_render_rtv.GetResource());
}


void TextRender::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb.SetName("Bright Lum pass volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	*/

	//uniformsCBData.scale = 1.0;

	auto fontTex = this->GetInput<1>().Get();

	commandList.SetResourceState(m_text_render_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(fontTex->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	RenderTargetView2D* pRTV = &m_text_render_rtv;
	commandList.SetRenderTargets(1, &pRTV, 0);

	gxapi::Rectangle rect{ 0, (int)m_text_render_rtv.GetResource().GetHeight(), 0, (int)m_text_render_rtv.GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder.value());
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder.value());
	commandList.BindGraphics(m_fontTexBindParam, fontTex->GetSrv());

	gxeng::VertexBuffer* pVertexBuffer = &m_fsq;
	unsigned vbSize = (unsigned)m_fsq.GetSize();
	unsigned vbStride = 5 * sizeof(float);

	commandList.SetResourceState(*pVertexBuffer, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
	commandList.SetResourceState(m_fsqIndices, gxapi::eResourceState::INDEX_BUFFER);
	commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
	commandList.SetIndexBuffer(&m_fsqIndices, false);

	std::string text = "Hello World!";
	uint32_t currPosX = 0;
	uint32_t currPosY = 0;

	float stepW = 1.0 / m_text_render_rtv.GetResource().GetWidth();
	float stepH = 1.0 / m_text_render_rtv.GetResource().GetHeight();


	for (int c = 0; c < text.size(); ++c)
	{
		int idx = text[c];
		uniformsCBData.x = characters[idx].x;
		uniformsCBData.y = m_texSizeH - characters[idx].y - characters[idx].h;
		uniformsCBData.w = characters[idx].w;
		uniformsCBData.h = characters[idx].h;

		Mat44 mulHalfAddHalf = 
		{
			0.5, 0, 0, 0,
			0, -0.5, 0, 0,
			0, 0, 1, 0,
			0.5, 0.5, 0, 1
		};

		Mat44 mulTwoSubOne =
		{
			2.0, 0, 0, 0,
			0, 2.0, 0, 0,
			0, 0, 1, 0,
			-1, -1, 0, 1
		};

		Mat44 scale = 
		{
			stepW * characters[idx].w, 0, 0, 0,
			0, stepH * characters[idx].h, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		float xTrans = stepW * (currPosX + characters[idx].xoff);
		float yTrans = stepH * (currPosY + characters[idx].yoff);
		Mat44 translate = 
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			xTrans, yTrans, 0, 1
		};

		uniformsCBData.trans = mulHalfAddHalf * scale * translate * mulTwoSubOne;

		uniformsCBData.color = Vec4(0.9, 0.1, 0.1, 1.0);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));
		commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());

		currPosX += characters[idx].xadvance;
	}
}


void TextRender::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;


		///////////////////////////////////////////////////////////////
		{ //http://www.angelcode.com/products/bmfont/doc/file_format.html#bin
			auto& fontBinary = *this->GetInput<2>().Get();

			//validate font binary data
			bool valid = true;
			//first 3 bytes spell 'BMF'
			valid = valid && fontBinary[0] == 66;
			valid = valid && fontBinary[1] == 77;
			valid = valid && fontBinary[2] == 70;
			//version must be 3
			valid = valid && fontBinary[3] == 3;
			assert(valid);

			characters.resize(256);

			//go over font binary info block-by-block
			int bytePtr = 4;
			while (bytePtr < fontBinary.size())
			{
				char blockType = fontBinary[bytePtr++];
				int blockSize = *(int*)&fontBinary[bytePtr];
				bytePtr += 4; //block starts here

				switch (blockType)
				{
				case 1:
				{ //info
					//ignored
					break;
				}
				case 2:
				{ //common
					//This is the distance in pixels between each line of text.
					m_lineHeight = *(uint16_t*)&fontBinary[bytePtr + 0];
					//The number of pixels from the absolute top of the line to the base of the characters.
					m_base = *(uint16_t*)&fontBinary[bytePtr + 2];
					//texture size
					m_texSizeW = *(uint16_t*)&fontBinary[bytePtr + 4];
					m_texSizeH = *(uint16_t*)&fontBinary[bytePtr + 6];
					break;
				}
				case 3:
				{ //pages
					//ignored, we are only going to support 1 texture page fonts
					break;
				}
				case 4:
				{ //chars
					m_numChars = blockSize / 20;

					for (int c = 0; c < m_numChars; ++c)
					{
						//The character id.
						uint32_t id = *(uint32_t*)&fontBinary[bytePtr + c * 20 + 0];
						characters[id] = *(character*)&fontBinary[bytePtr + c * 20 + 4];
					}
					break;
				}
				case 5:
				{ //kerning pairs
					//ignored
					break;
				}
				default: break;
				}


				bytePtr += blockSize;
			}
		}
		///////////////////////////////////////////////////////////////
	}
}


} // namespace inl::gxeng::nodes
