#include "RenderForwardSimple.hpp"

#include "../Mesh.hpp"
#include "../Material.hpp"
#include "../MaterialShader.hpp"

#include "../MeshEntity.hpp"

#include <BaseLibrary/Range.hpp>


namespace inl::gxeng::nodes {



auto PipelineStateManager::GetPipelineState(RenderContext& context, const Mesh& mesh, const Material& material) -> const StateDesc& {
	std::string vsStr = GenerateVertexShader(context, mesh);
	std::string mtlStr = GenerateMaterialShader(material);
	std::string psStr = GeneratePixelShader(context, material);

	auto pos = psStr.find_first_of("//!1");
	assert(pos != psStr.npos);
	for (auto i : Range(pos, pos + 4)) {
		psStr[i] = ' ';
	}

	pos = psStr.find_first_of("################");
	assert(pos != psStr.npos);
	std::stringstream ss;
	ss << std::hex << std::setw(16) << material.GetShader()->GetId().Value();
	std::string serial = ss.str();
	for (auto i : Range(pos, pos + 16)) {
		psStr[i] = serial[i-pos];
	}

	context.StoreShader("material_shader_" + serial, mtlStr);

	std::string macros;
	bool hasPos;
	for (auto stream : Range(mesh.GetLayout().GetStreamCount())) {
		for (auto element : mesh.GetLayout()[stream]) {
			if (element.semantic == eVertexElementSemantic::POSITION && element.index == 0) {
				hasPos = true;
			}
			if (element.semantic == eVertexElementSemantic::NORMAL && element.index == 0) {
				macros += "HAS_NORMAL ";
			}
			if (element.semantic == eVertexElementSemantic::TANGENT && element.index == 0) {
				macros += "HAS_TANGENT ";
			}
			if (element.semantic == eVertexElementSemantic::BITANGENT && element.index == 0) {
				macros += "HAS_BITANGENT ";
			}
			if (element.semantic == eVertexElementSemantic::COLOR && element.index == 0) {
				macros += "HAS_COLOR ";
			}
			if (element.semantic == eVertexElementSemantic::TEX_COORD && element.index == 0) {
				macros += "HAS_TEXCOORD ";
			}
		}
	}
	if (!hasPos) {
		throw InvalidArgumentException("Vertex must have at least position attribute.");
	}

	ShaderParts parts;
	parts.vs = true;
	ShaderProgram vs = context.CompileShader(vsStr, parts, macros);
	parts.vs = false;
	parts.ps = true;
	ShaderProgram ps = context.CompileShader(psStr, parts, macros);
}

std::string PipelineStateManager::GenerateVertexShader(RenderContext& context, const Mesh& mesh) {
	const auto& layout = mesh.GetLayout();

	return context.LoadShader("ForwardRenderSimple");
}

std::string PipelineStateManager::GenerateMaterialShader(const Material& material) {
	const MaterialShader& shader = *material.GetShader();

	std::stringstream mtlConstantBuffer;
	std::stringstream mtlTextures;
	std::stringstream mtlMain;

	mtlConstantBuffer << "struct MtlConstants { \n";
	int numMtlConstants = 0;
	for (size_t i = 0; i < material.GetParameterCount(); ++i) {
		switch (material[i].GetType()) {
			case eMaterialShaderParamType::COLOR:
			{
				mtlConstantBuffer << "    float4 param" << i << "; \n";
				++numMtlConstants;
				break;
			}
			case eMaterialShaderParamType::VALUE:
			{
				mtlConstantBuffer << "    float param" << i << "; \n";
				++numMtlConstants;
				break;
			}
			case eMaterialShaderParamType::BITMAP_COLOR_2D:
			{
				mtlTextures << "Texture2DArray<float4> tex" << i << " : register(t" << i << "); \n";
				mtlTextures << "SamplerState samp" << i << " : register(s" << i << "); \n";
				break;
			}
			case eMaterialShaderParamType::BITMAP_VALUE_2D:
			{
				mtlTextures << "Texture2DArray<float> tex" << i << " : register(t" << i << "); \n";
				mtlTextures << "SamplerState samp" << i << " : register(s" << i << "); \n";
				break;
			}
		}
	}

	mtlMain << "void MtlMain() {\n";
	for (size_t i = 0; i < material.GetParameterCount(); ++i) {
		switch (material[i].GetType()) {
			case eMaterialShaderParamType::COLOR:
			{
				mtlMain << "    float4 input" << i << "; \n";
				mtlMain << "    input" << i << " = mtlCb.param" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::VALUE:
			{
				mtlMain << "    float input" << i << "; \n";
				mtlMain << "    input" << i << " = mtlCb.param" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::BITMAP_COLOR_2D:
			{
				mtlMain << "    MapColor2D input" << i << "; \n";
				mtlMain << "    input" << i << ".tex = tex" << i << "; \n";
				mtlMain << "    input" << i << ".samp = samp" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::BITMAP_VALUE_2D:
			{
				mtlMain << "    MapValue2D input" << i << "; \n";
				mtlMain << "    input" << i << ".tex = tex" << i << "; \n";
				mtlMain << "    input" << i << ".samp = samp" << i << "; \n\n";
				break;
			}
		}
	}

	size_t numOutParams = 0;
	for (auto i : Range(shader.GetNumOutputs())) {
		if (shader.GetOutput(i)->index >= 0) {
			mtlMain << "    " << shader.GetOutput(i)->GetType() << " " << shader.GetOutput(i)->GetName() << ";\n";
			++numOutParams;
		}
	}

	size_t numParams = shader.GetNumInputs() + numOutParams;
	std::vector<std::string> params(numParams);

	for (auto i : Range(shader.GetNumOutputs())) {
		if (shader.GetOutput(i)->index >= 0) {
			params[shader.GetOutput(i)->index] = "output" + std::to_string(i);
		}
	}

	for (auto i : Range(material.GetParameterCount())) {
		params[material[i].GetShaderParamIndex()] = "input" + std::to_string(i);
	}

	for (auto i : Range(shader.GetNumInputs())) {
		params[shader.GetInput(i)->index] = shader.GetInput(i)->GetDefaultValue();
	}


	mtlMain << "mtl_shader(";
	for (intptr_t i = 0; i < numParams; ++i) {
		mtlMain << params[i];
		if (i < (intptr_t)numParams - 1) {
			mtlMain << ", ";
		}
	}
	mtlMain << ");\n";

	mtlMain << "}\n\n";

	std::string mtlShaderCall =
		mtlConstantBuffer.str() + mtlTextures.str() + mtlMain.str();

	return mtlShaderCall;
}

std::string PipelineStateManager::GeneratePixelShader(RenderContext & context, const Material & shader) {
	return context.LoadShader("ForwardRenderSimple");
}


void RenderForwardSimple::Reset() {
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}


void RenderForwardSimple::Setup(SetupContext& context) {
	auto renderTarget = GetInput<0>().Get();
	auto depthTarget = GetInput<1>().Get();
	auto camera = GetInput<2>().Get();
	auto entities = GetInput<3>().Get();
	auto directionalLight = GetInput<4>().Get();


}


void RenderForwardSimple::Execute(RenderContext& context) {
	auto renderTarget = GetInput<0>().Get();
	auto depthTarget = GetInput<1>().Get();
	auto camera = GetInput<2>().Get();
	auto entities = GetInput<3>().Get();
	auto directionalLight = GetInput<4>().Get();

	for (auto& entity : *entities) {
		if (!entity->GetMesh() || !entity->GetMaterial()) {
			continue;
		}
		const Mesh& mesh = *entity->GetMesh();
		const Material& material = *entity->GetMaterial();

		m_psoManager.GetPipelineState(context, mesh, material);
	}
}



const std::string& RenderForwardSimple::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
		"Camera",
		"Entities",
		"Lights",
	};
	return names[index];
}


const std::string& RenderForwardSimple::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
	};
	return names[index];
}





} // namespace inl::gxeng
