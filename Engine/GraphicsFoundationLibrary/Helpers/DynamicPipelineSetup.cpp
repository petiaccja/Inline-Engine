#include "DynamicPipelineSetup.hpp"

#include <BaseLibrary/BitOperations.hpp>
#include <BaseLibrary/Range.hpp>
#include <GraphicsEngine/Resources/Vertex.hpp>
#include <GraphicsEngine_LL/Material.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>

#include <iomanip>
#include <regex>


namespace inl::gxeng::nodes {


size_t MaterialParamSize(eMaterialShaderParamType type) {
	switch (type) {
		case eMaterialShaderParamType::COLOR: return 16; // sizeof(float4)
		case eMaterialShaderParamType::VALUE: return 4; // sizeof(float)
		default: return 0;
	}
}

size_t Alignment(size_t objectSize) {
	assert(objectSize != 0);
	int zeros = CountLeadingZeros(objectSize);
	return ~(size_t(-1) >> 1) >> (zeros - 1);
}

size_t Pad(size_t size, size_t alignment) {
	return ((size + alignment - 1) / alignment) * alignment;
}

std::tuple<size_t, std::vector<MaterialCbufferElement>> MaterialCbuffer(const Material& material) {
	std::vector<MaterialCbufferElement> elements;

	int cbSize = 0;
	for (auto i : Range(material.GetParameterCount())) {
		size_t paramSize = MaterialParamSize(material[i].GetType());
		size_t offset = cbSize;
		if (paramSize > 0) {
			size_t paramAlignment = Alignment(paramSize);
			cbSize = Pad(cbSize, paramAlignment) + paramSize;
		}
		elements.push_back(MaterialCbufferElement{ .structureIndex = 0, .paramIndex = i, .offset = offset, .size = paramSize, .optionalOffset = 0 });
		size_t optionalOffset = cbSize;
		if (material[i].IsOptional()) {
			// Add an extra bool bool, which is 32 bits in HLSL.
			cbSize = Pad(cbSize, Alignment(4)) + 4;
			elements.push_back(MaterialCbufferElement{ .structureIndex = 0, .paramIndex = i, .offset = offset, .size = paramSize, .optionalOffset = optionalOffset });
		}
	}
	return { cbSize, elements };
}

std::vector<BindParameterDesc> MaterialConstantBinding(const Material& material, unsigned baseRegister) {
	auto [size, _elements] = MaterialCbuffer(material);

	BindParameterDesc desc;
	desc.constantSize = size;
	desc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;
	desc.parameter.reg = baseRegister;
	desc.parameter.space = 0;
	desc.parameter.type = eBindParameterType::CONSTANT;
	return { desc };
}

std::vector<BindParameterDesc> MaterialTextureBinding(const Material& material, unsigned baseRegister) {
	std::vector<BindParameterDesc> descs;

	size_t reg = 0;
	for (auto i : Range(material.GetParameterCount())) {
		auto type = material[i].GetType();
		if (type == eMaterialShaderParamType::BITMAP_COLOR_2D || type == eMaterialShaderParamType::BITMAP_VALUE_2D) {
			BindParameterDesc texParam;
			texParam.parameter.reg = baseRegister + reg++;
			texParam.parameter.type = eBindParameterType::TEXTURE;
			texParam.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

			descs.push_back(texParam);
		}
	}

	return descs;
}


std::vector<gxapi::InputElementDesc> MeshInputElements(const Mesh& mesh) {
	const Mesh::Layout& layout = mesh.GetLayout();
	std::vector<gxapi::InputElementDesc> inputElements;

	for (auto streamIdx : Range(layout.GetStreamCount())) {
		for (const auto& element : layout[streamIdx]) {
			if (element.index != 0) {
				continue;
			}
			switch (element.semantic) {
				case eVertexElementSemantic::POSITION:
					inputElements.push_back({ "POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, unsigned(streamIdx), unsigned(element.offset )});
					break;
				case eVertexElementSemantic::NORMAL:
					inputElements.push_back({"NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, unsigned(streamIdx), unsigned(element.offset)});
					break;
				case eVertexElementSemantic::COLOR:
					inputElements.push_back({"COLOR", 0, gxapi::eFormat::R32G32B32A32_FLOAT, unsigned(streamIdx), unsigned(element.offset)});
					break;
				case eVertexElementSemantic::TEX_COORD:
					inputElements.push_back({"TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, unsigned(streamIdx), unsigned(element.offset)});
					break;
				case eVertexElementSemantic::TANGENT:
					inputElements.push_back({"TANGENT", 0, gxapi::eFormat::R32G32B32_FLOAT, unsigned(streamIdx), unsigned(element.offset)});
					break;
				case eVertexElementSemantic::BITANGENT:
					inputElements.push_back({ "BITANGENT", 0, gxapi::eFormat::R32G32B32_FLOAT, unsigned(streamIdx), unsigned(element.offset )});
					break;
			}
		}
	}

	return inputElements;
}



std::vector<std::string> MeshLayoutMacros(const Mesh& mesh) {
	std::vector<std::string> macros;
	for (auto stream : Range(mesh.GetLayout().GetStreamCount())) {
		for (auto element : mesh.GetLayout()[stream]) {
			if (element.semantic == eVertexElementSemantic::NORMAL && element.index == 0) {
				macros.push_back("HAS_NORMAL=1");
			}
			if (element.semantic == eVertexElementSemantic::TANGENT && element.index == 0) {
				macros.push_back("HAS_TANGENT=1");
			}
			if (element.semantic == eVertexElementSemantic::BITANGENT && element.index == 0) {
				macros.push_back("HAS_BITANGENT=1");
			}
			if (element.semantic == eVertexElementSemantic::COLOR && element.index == 0) {
				macros.push_back("HAS_COLOR=1");
			}
			if (element.semantic == eVertexElementSemantic::TEX_COORD && element.index == 0) {
				macros.push_back("HAS_TEXCOORD=1");
			}
		}
	}
	return macros;
}


size_t BaseConstantRegister(const std::vector<BindParameterDesc>& params, int space) {
	size_t maxRegister = 0;
	constexpr size_t registerSize = 16;
	for (auto& param : params) {
		if (param.parameter.space == space && param.parameter.type == eBindParameterType::CONSTANT) {
			size_t endRegister = param.parameter.reg + Pad(param.constantSize, registerSize) / registerSize;
			maxRegister = std::max(maxRegister, endRegister);
		}
	}
	return maxRegister;
}


size_t BaseTextureRegister(const std::vector<BindParameterDesc>& params, int space) {
	// Note this doesn't properly handle texture arrays.
	size_t maxRegister = 0;
	constexpr size_t registerSize = 16;
	for (auto& param : params) {
		if (param.parameter.space == space && param.parameter.type == eBindParameterType::TEXTURE) {
			maxRegister = std::max(maxRegister, (size_t)param.parameter.reg);
		}
	}
	return maxRegister + 1;
}


std::string MaterialConstantsStructCode(const Material& material, std::string_view structName) {
	std::stringstream mtlConstantBuffer;
	mtlConstantBuffer << "struct " << structName << " { \n";
	int numMtlConstants = 0;
	int textureReg = 0;
	for (size_t i = 0; i < material.GetParameterCount(); ++i) {
		auto& param = material[i];
		switch (param.GetType()) {
			case eMaterialShaderParamType::COLOR: {
				mtlConstantBuffer << "    float4 param" << i << ";\n";
				++numMtlConstants;
				break;
			}
			case eMaterialShaderParamType::VALUE: {
				mtlConstantBuffer << "    float param" << i << ";\n";
				++numMtlConstants;
				break;
			}
		}

		if (param.IsOptional()) {
			mtlConstantBuffer << "    bool param" << i << "_opt;\n";
		}
	}
	mtlConstantBuffer << "};\n";
	return mtlConstantBuffer.str();
}


std::string MaterialConstantsDeclarationCode(size_t baseRegister, std::string_view structName, std::string_view varName) {
	std::stringstream constantDeclarations;
	constantDeclarations << "ConstantBuffer<" << structName << "> "
						 << varName
						 << " : register(b" << baseRegister << ");\n";
	return constantDeclarations.str();
}

std::string MaterialTexturesDeclarationCode(const Material& material, size_t baseRegister, std::string_view varName) {
	std::stringstream textureDeclarations;
	for (size_t i = 0; i < material.GetParameterCount(); ++i) {
		auto& param = material[i];
		switch (param.GetType()) {
			case eMaterialShaderParamType::BITMAP_COLOR_2D: {
				textureDeclarations << "Texture2DArray<float4> "
									<< varName << i
									<< " : register(t" << baseRegister << ");\n";
				++baseRegister;
				break;
			}
			case eMaterialShaderParamType::BITMAP_VALUE_2D: {
				textureDeclarations << "Texture2DArray<float> "
									<< varName << i
									<< " : register(t" << baseRegister << ");\n";
				++baseRegister;
				break;
			}
		}
	}
	return textureDeclarations.str();
}

std::string MaterialLoadParamsCode(const Material& material, std::string_view constVarName, std::string_view texVarName, std::string_view samplerName) {
	std::stringstream loadParams;

	for (size_t i = 0; i < material.GetParameterCount(); ++i) {
		auto& param = material[i];
		auto& input = *material.GetShader()->GetInput(param.GetShaderParamIndex());

		std::stringstream ssOptOpen;
		std::stringstream ssOptClose;
		if (param.IsOptional()) {
			ssOptOpen << "    if (" << constVarName << ".param" << i << "_opt) {\n    ";
			ssOptClose << "    } else {\n";
			ssOptClose << "        input" << i << " = " << input.GetDefaultValue() << ";\n";
			ssOptClose << "    }\n\n";
		}

		switch (param.GetType()) {
			case eMaterialShaderParamType::COLOR: {
				loadParams << "    float4 input" << i << "; \n";
				loadParams << ssOptOpen.str() << "    input" << i << " = " << constVarName << ".param" << i << "; \n"
						   << ssOptClose.str();
				break;
			}
			case eMaterialShaderParamType::VALUE: {
				loadParams << "    float input" << i << "; \n";
				loadParams << ssOptOpen.str() << "    input" << i << " = " << constVarName << ".param" << i << "; \n"
						   << ssOptClose.str();
				break;
			}
			case eMaterialShaderParamType::BITMAP_COLOR_2D: {
				loadParams << "    MapColor2D input" << i << "; \n";
				loadParams << ssOptOpen.str();
				loadParams << "    input" << i << ".tex = " << texVarName << i << "; \n";
				loadParams << "    input" << i << ".samp = " << samplerName
						   << "; \n\n";
				loadParams << ssOptClose.str();
				break;
			}
			case eMaterialShaderParamType::BITMAP_VALUE_2D: {
				loadParams << "    MapValue2D input" << i << "; \n";
				loadParams << ssOptOpen.str();
				loadParams << "    input" << i << ".tex = " << texVarName << i << "; \n";
				loadParams << "    input" << i << ".samp = " << samplerName
						   << "; \n\n";
				loadParams << ssOptClose.str();
				break;
			}
		}
	}

	return loadParams.str();
}

std::string MaterialOutputDeclarationsCode(const Material& material) {
	std::stringstream outputDeclarations;

	auto& shader = *material.GetShader();
	for (auto i : Range(shader.GetNumOutputs())) {
		if (shader.GetOutput(i)->index >= 0) {
			outputDeclarations << "    " << shader.GetOutput(i)->GetType() << " " << shader.GetOutput(i)->GetName() << ";\n";
		}
	}

	return outputDeclarations.str();
}


std::string MaterialMainCode(const Material& material) {
	auto& shader = *material.GetShader();

	// Number of output arguments of the main(...) material call.
	size_t numOutputArgs = 0;
	for (auto i : Range(shader.GetNumOutputs())) {
		if (shader.GetOutput(i)->index >= 0) { // Exclude return value.
			++numOutputArgs;
		}
	}

	// Textual representation of the comma-separated argument list of the main(...) call.
	std::vector<std::string> arguments(numOutputArgs + shader.GetNumInputs());

	// Set the arguments from the "output declarations" function above.
	for (auto i : Range(shader.GetNumOutputs())) {
		if (shader.GetOutput(i)->index >= 0) {
			arguments[shader.GetOutput(i)->index] = shader.GetOutput(i)->GetName();
		}
	}

	// Set all the input arguments to the default value coming from MaterialShader.
	for (auto i : Range(shader.GetNumInputs())) {
		arguments[shader.GetInput(i)->index] = shader.GetInput(i)->GetDefaultValue();
	}

	// Overwrite some of the default input arguments with inputs coming from the Material.
	// Note: a MaterialShader might provide legible default values for input parameters
	// that are not a legible type for the Material itself and cannot be set from C++, only
	// from the shader directly.
	for (auto i : Range(material.GetParameterCount())) {
		arguments[material[i].GetShaderParamIndex()] = "input" + std::to_string(i);
	}

	std::stringstream shaderCall;
	shaderCall << "    main(";
	for (intptr_t i = 0; i < arguments.size(); ++i) {
		shaderCall << arguments[i];
		if (i < (intptr_t)arguments.size() - 1) {
			shaderCall << ", ";
		}
	}
	shaderCall << ");\n";

	return shaderCall.str();
}


std::string MaterialCode(const Material& material, std::string_view materialMainName, size_t baseConstantReg, size_t baseTextureReg, std::string_view materialSamplerName) {
	std::stringstream materialCode;

	materialCode << material.GetShader()->GetShaderCode() << "\n\n"
				 << MaterialConstantsStructCode(material, "MaterialConstants") << "\n"
				 << MaterialConstantsDeclarationCode(baseConstantReg, "MaterialConstants", "materialConstants") << "\n"
				 << MaterialTexturesDeclarationCode(material, baseTextureReg, "materialTexture") << "\n";

	materialCode << "void " << materialMainName << "() {\n"
				 << MaterialLoadParamsCode(material, "materialConstants", "materialTexture", materialSamplerName) << "\n"
				 << MaterialOutputDeclarationsCode(material) << "\n"
				 << MaterialMainCode(material) << "\n"
				 << "}\n\n";

	return materialCode.str();
}


PipelineSetupDesc PipelineSetup(PipelineSetupTemplate base, const Mesh& mesh, const Material& material) {
	size_t baseConstantReg = BaseConstantRegister(base.bindParams, 0);
	size_t baseTextureReg = BaseTextureRegister(base.bindParams, 0);

	PipelineSetupDesc desc;

	desc.materialConstantParams = MaterialConstantBinding(material, baseConstantReg);
	desc.materialTextureParams = MaterialTextureBinding(material, baseTextureReg);
	desc.inputLayout = MeshInputElements(mesh);
	desc.materialCode = MaterialCode(material, base.materialMainName, baseConstantReg, baseTextureReg, base.materialSamplerName);

	auto [_size, elements] = MaterialCbuffer(material);
	desc.materialConstantElements = std::move(elements);

	std::vector<std::string> layoutMacros = MeshLayoutMacros(mesh);

	desc.vsMacros = layoutMacros;
	desc.gsMacros = layoutMacros;
	desc.hsMacros = layoutMacros;
	desc.dsMacros = layoutMacros;
	desc.psMacros = layoutMacros;
	desc.vsMacros.push_back("VERTEX_SHADER");
	desc.gsMacros.push_back("GEOMETRY_SHADER");
	desc.hsMacros.push_back("HULL_SHADER");
	desc.dsMacros.push_back("DOMAIN_SHADER");
	desc.psMacros.push_back("PIXEL_SHADER");

	return desc;
}


} // namespace inl::gxeng::nodes
