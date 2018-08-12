#pragma once

#include <InlineMath.hpp>
#include <vector>
#include <unordered_map>


namespace inl::gxeng {

class Image;
class MaterialShader;


enum class eMaterialShaderParamType {
	COLOR = 0,
	VALUE = 1,
	BITMAP_COLOR_2D = 2,
	BITMAP_VALUE_2D = 3,
	UNKNOWN = 1000,
};


class Material {
public:
	class Parameter {
	public:
		Parameter();
		Parameter(std::string name, eMaterialShaderParamType type, int shaderParamIndex, bool optional);

		const std::string& GetName() const;
		eMaterialShaderParamType GetType() const;
		int GetShaderParamIndex() const;

		Parameter& operator=(Image*);
		Parameter& operator=(Vec4);
		Parameter& operator=(float);

		operator Image*() const;
		operator Vec4() const;
		operator float() const;

		bool IsSet() const { return m_set; }
		bool IsOptional() const { return m_optional; }
	private:
		std::string m_name;
		eMaterialShaderParamType m_type;
		int m_shaderParamIndex;
		union Data {
			Data() { memset(this, 0, sizeof(*this)); }
			Data(const Data& rhs) { memcpy(this, &rhs, sizeof(*this)); }
			Data& operator=(const Data& rhs) { memcpy(this, &rhs, sizeof(*this)); return *this; }
			Image* image;
			Vec4 color;
			float value;
		} m_data;
		bool m_set = false;
		bool m_optional = false;
	};

public:
	void SetShader(const MaterialShader* shader);
	const MaterialShader* GetShader() const;
	size_t GetParameterCount() const;

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