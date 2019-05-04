#include "Material.hpp"

#include "MaterialShader.hpp"

#include <BaseLibrary/Exception/Exception.hpp>
#include <BaseLibrary/Range.hpp>


namespace inl::gxeng {



static eMaterialShaderParamType GetParameterType(std::string typeString) {
	if (typeString == "float4") {
		return eMaterialShaderParamType::COLOR;
	}
	else if (typeString == "float") {
		return eMaterialShaderParamType::VALUE;
	}
	else if (typeString == "MapColor2D") {
		return eMaterialShaderParamType::BITMAP_COLOR_2D;
	}
	else if (typeString == "MapValue2D") {
		return eMaterialShaderParamType::BITMAP_VALUE_2D;
	}
	else {
		return eMaterialShaderParamType::UNKNOWN;
	}
}


void Material::SetShader(const MaterialShader* shader) {
	std::vector<Parameter> parameters;
	std::unordered_map<std::string, size_t> paramNameMap;

	int paramIdx = 0;
	for (auto inputIdx : Range(shader->GetNumInputs())) {
		const MaterialShaderInput* input = shader->GetInput(inputIdx);
		bool mandatory = input->GetDefaultValue().empty();
		eMaterialShaderParamType type = GetParameterType(input->type);
		if (type == eMaterialShaderParamType::UNKNOWN && mandatory) {
			std::stringstream ssextra;
			ssextra << "You provided \"" << input->type << "\" for parameter " << inputIdx << ".";
			ssextra << "See eMaterialShaderParamType for acceptable values.";
			throw InvalidArgumentException(
				"Material shader can only be used with a material if its input ports are of specific type.",
				ssextra.str());
		}
		if (type == eMaterialShaderParamType::UNKNOWN) {
			continue;
		}
		parameters.emplace_back(input->name, type, (int)inputIdx, !mandatory);
		paramNameMap.insert_or_assign(input->name, paramIdx);
		++paramIdx;
	}

	m_parameters = std::move(parameters);
	m_paramNameMap = std::move(paramNameMap);
	m_shader = shader;
}

const MaterialShader* Material::GetShader() const {
	return m_shader;
}


size_t Material::GetParameterCount() const {
	return m_parameters.size();
}


IMaterial::Parameter& Material::GetParameter(size_t index) {
	return operator[](index);
}


const IMaterial::Parameter& Material::GetParameter(size_t index) const {
	return operator[](index);
}


IMaterial::Parameter& Material::GetParameter(const std::string& name) {
	return operator[](name);
}


const IMaterial::Parameter& Material::GetParameter(const std::string& name) const {
	return operator[](name);
}


Material::Parameter& Material::operator[](size_t index) {
	return m_parameters[index];
}
const Material::Parameter& Material::operator[](size_t index) const {
	return m_parameters[index];
}


Material::Parameter& Material::operator[](const std::string& name) {
	auto it = m_paramNameMap.find(name);
	if (it == m_paramNameMap.end()) {
		throw InvalidArgumentException("Material input with given name cannot be found.", name);
	}
	return (*this)[it->second];
}
const Material::Parameter& Material::operator[](const std::string& name) const {
	auto it = m_paramNameMap.find(name);
	if (it == m_paramNameMap.end()) {
		throw InvalidArgumentException("Material input with given name cannot be found.", name);
	}
	return (*this)[it->second];
}


} // namespace inl::gxeng