#include "Material2.hpp"
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



//------------------------------------------------------------------------------
// Material
//------------------------------------------------------------------------------


void Material2::SetShader(const MaterialShader2* shader) {
	std::vector<Parameter> parameters;
	std::unordered_map<std::string, size_t> paramNameMap;

	for (auto inputIdx : Range(shader->GetNumInputs())) {
		const MaterialShaderInput* input = shader->GetInput(inputIdx);
		eMaterialShaderParamType type = GetParameterType(input->type);
		if (type == eMaterialShaderParamType::UNKNOWN) {
			std::stringstream ssextra;
			ssextra << "You provided \"" << input->type << "\" for parameter " << inputIdx << ".";
			ssextra << "See eMaterialShaderParamType for acceptable values.";
			throw InvalidArgumentException(
				"Material shader can only be used with a material if its input ports are of specific type.",
				ssextra.str()
			);
		}
		parameters.emplace_back(type);
		paramNameMap.insert_or_assign(input->name, inputIdx);
	}

	m_parameters = std::move(parameters);
	m_paramNameMap = std::move(paramNameMap);
}

const MaterialShader2* Material2::GetShader() const {
	return m_shader;
}


size_t Material2::GetParameterCount() const {
	return m_parameters.size();
}


Material2::Parameter& Material2::operator[](size_t index) {
	return m_parameters[index];
}
const Material2::Parameter& Material2::operator[](size_t index) const {
	return m_parameters[index];
}


Material2::Parameter& Material2::operator[](const std::string& name) {
	auto it = m_paramNameMap.find(name);
	assert(it != m_paramNameMap.end());
	return (*this)[it->second];
}
const Material2::Parameter& Material2::operator[](const std::string& name) const {
	auto it = m_paramNameMap.find(name);
	assert(it != m_paramNameMap.end());
	return (*this)[it->second];
}



//------------------------------------------------------------------------------
// Material::Parameter
//------------------------------------------------------------------------------

Material2::Parameter::Parameter() {
	m_type = eMaterialShaderParamType::UNKNOWN;
}
Material2::Parameter::Parameter(eMaterialShaderParamType type) {
	m_type = type;
}


Material2::Parameter& Material2::Parameter::operator=(Image* image) {
	if (m_type != eMaterialShaderParamType::BITMAP_COLOR_2D && m_type != eMaterialShaderParamType::BITMAP_VALUE_2D) {
		throw InvalidArgumentException("This parameter is not an image.");
	}

	m_data.image = image;
	return *this;
}

Material2::Parameter& Material2::Parameter::operator=(Vec4 color) {
	if (m_type != eMaterialShaderParamType::COLOR) {
		throw InvalidArgumentException("This parameter is not a color.");
	}

	m_data.color = color;
	return *this;
}

Material2::Parameter& Material2::Parameter::operator=(float value) {
	if (m_type != eMaterialShaderParamType::VALUE) {
		throw InvalidArgumentException("This parameter is not a value.");
	}

	m_data.value = value;
	return *this;
}


eMaterialShaderParamType Material2::Parameter::GetType() const {
	return m_type;
}


Material2::Parameter::operator Image*() const {
	if (m_type != eMaterialShaderParamType::BITMAP_COLOR_2D && m_type != eMaterialShaderParamType::BITMAP_VALUE_2D) {
		throw InvalidArgumentException("This parameter is not an image.");
	}
	return m_data.image;
}

Material2::Parameter::operator Vec4() const {
	if (m_type != eMaterialShaderParamType::COLOR) {
		throw InvalidArgumentException("This parameter is not a color.");
	}
	return m_data.color;
}

Material2::Parameter::operator float() const {
	if (m_type != eMaterialShaderParamType::VALUE) {
		throw InvalidArgumentException("This parameter is not a value.");
	}
	return m_data.value;
}



} // namespace inl::gxeng