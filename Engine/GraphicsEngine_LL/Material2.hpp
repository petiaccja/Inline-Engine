#pragma once

#include <InlineMath.hpp>
#include <vector>
#include <unordered_map>


namespace inl::gxeng {

class Image;
class MaterialShader2;


enum class eMaterialShaderParamType {
	COLOR = 0,
	VALUE = 1,
	BITMAP_COLOR_2D = 2,
	BITMAP_VALUE_2D = 3,
	UNKNOWN = 1000,
};


class Material2 {
public:
	class Parameter {
	public:
		Parameter();
		Parameter(eMaterialShaderParamType type);

		Parameter& operator=(Image*);
		Parameter& operator=(Vec4);
		Parameter& operator=(float);

		eMaterialShaderParamType GetType() const;
		operator Image*() const;
		operator Vec4() const;
		operator float() const;
	private:
		eMaterialShaderParamType m_type;
		union Data {
			Data() { memset(this, 0, sizeof(*this)); }
			Data(const Data& rhs) { memcpy(this, &rhs, sizeof(*this)); }
			Data& operator=(const Data& rhs) { memcpy(this, &rhs, sizeof(*this)); return *this; }
			Image* image;
			Vec4 color;
			float value;
		} m_data;
	};

public:
	void SetShader(const MaterialShader2* shader);
	const MaterialShader2* GetShader() const;
	size_t GetParameterCount() const;

	Parameter& operator[](size_t index);
	const Parameter& operator[](size_t index) const;

	Parameter& operator[](const std::string& name);
	const Parameter& operator[](const std::string& name) const;
private:
	std::vector<Parameter> m_parameters;
	const MaterialShader2* m_shader = nullptr;
	std::unordered_map<std::string, size_t> m_paramNameMap; // maps parameter names to indices
};



} // namespace inl::gxeng