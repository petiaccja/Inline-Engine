#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/Binder.hpp>


namespace inl::gxeng {
class Mesh;
class Material;
} // namespace inl::gxeng


namespace inl::gxeng::nodes {



struct MaterialCbufferElement {
	size_t structureIndex = 0;
	size_t paramIndex = 0;
	size_t offset = 0;
	size_t size = 0;
	size_t optionalOffset = 0;
};

struct PipelineSetupTemplate {
	std::vector<BindParameterDesc> bindParams;
	std::string materialSamplerName;
	std::string materialMainName;
};


struct PipelineSetupDesc {
	std::vector<BindParameterDesc> materialConstantParams;
	std::vector<BindParameterDesc> materialTextureParams;
	std::vector<MaterialCbufferElement> materialConstantElements;

	std::string materialCode;

	std::vector<std::string> vsMacros;
	std::vector<std::string> gsMacros;
	std::vector<std::string> hsMacros;
	std::vector<std::string> dsMacros;
	std::vector<std::string> psMacros;

	std::vector<gxapi::InputElementDesc> inputLayout;
};


PipelineSetupDesc PipelineSetup(PipelineSetupTemplate base, const Mesh& mesh, const Material& material);



class PipelineSetup {
};


} // namespace inl::gxeng::nodes