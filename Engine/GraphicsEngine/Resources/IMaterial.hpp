#pragma once


#include <InlineMath.hpp>
#include <string>
#include <variant>


namespace inl::gxeng {


class IImage;
class IMaterialShader;


enum class eMaterialShaderParamType {
	COLOR = 0,
	VALUE = 1,
	BITMAP_COLOR_2D = 2,
	BITMAP_VALUE_2D = 3,
	UNKNOWN = 1000,
};


class IMaterial {
public:
	class Parameter {
	public:
		Parameter() = default;
		Parameter(std::string name, eMaterialShaderParamType type, int shaderParamIndex, bool optional)
			: m_name(name), m_type(type), m_shaderParamIndex(shaderParamIndex), m_optional(optional) {}

		const std::string& GetName() const { return m_name; }
		eMaterialShaderParamType GetType() const { return m_type; }
		int GetShaderParamIndex() const { return m_shaderParamIndex; }

		Parameter& operator=(IImage* image) {
			m_data = image;
			return *this;
		}
		Parameter& operator=(Vec4 color) {
			m_data = color;
			return *this;
		}
		Parameter& operator=(float value) {
			m_data = value;
			return *this;
		}

		template <class Image, class = std::enable_if_t<std::is_base_of_v<IImage, Image>>>
		operator Image*() const { return static_cast<Image*>(std::get<IImage*>(m_data)); }
		operator IImage*() const { return std::get<IImage*>(m_data); }
		operator Vec4() const { return std::get<Vec4>(m_data); }
		operator float() const { return std::get<float>(m_data); }

		bool IsSet() const { return !std::holds_alternative<std::monostate>(m_data); }
		bool IsOptional() const { return m_optional; }

	private:
		std::string m_name;
		eMaterialShaderParamType m_type = eMaterialShaderParamType::UNKNOWN;
		int m_shaderParamIndex = 0;
		std::variant<std::monostate, IImage*, Vec4, float> m_data;
		bool m_optional = false;
	};

public:
	virtual void SetShader(const IMaterialShader* shader) = 0;
	virtual const IMaterialShader* GetShader() const = 0;
	virtual size_t GetParameterCount() const = 0;

	virtual Parameter& GetParameter(size_t index) = 0;
	virtual const Parameter& GetParameter(size_t index) const = 0;

	virtual Parameter& GetParameter(const std::string& name) = 0;
	virtual const Parameter& GetParameter(const std::string& name) const = 0;
};


} // namespace inl::gxeng