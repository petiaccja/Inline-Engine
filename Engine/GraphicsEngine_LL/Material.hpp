#pragma once

#include "MaterialShader.hpp"

#include <GraphicsEngine/Resources/IMaterial.hpp>

#include <string>
#include <unordered_map>
#include <vector>


namespace inl::gxeng {

class Image;
class MaterialShader;



class Material : public IMaterial {
public:
	using IMaterial::Parameter;

public:
	void SetShader(const MaterialShader* shader);
	void SetShader(const IMaterialShader* shader) { SetShader(dynamic_cast<const MaterialShader*>(shader)); }
	const MaterialShader* GetShader() const override;
	size_t GetParameterCount() const override;

	Parameter& GetParameter(size_t index) override;
	const Parameter& GetParameter(size_t index) const override;

	Parameter& GetParameter(const std::string& name) override;
	const Parameter& GetParameter(const std::string& name) const override;

	Parameter& operator[](size_t index);
	const Parameter& operator[](size_t index) const;

	Parameter& operator[](const std::string& name);
	const Parameter& operator[](const std::string& name) const;

private:
	std::vector<Parameter> m_parameters;
	const MaterialShader* m_shader = nullptr;
	std::unordered_map<std::string, size_t> m_paramNameMap; // maps parameter names to indices
};



} // namespace inl::gxeng