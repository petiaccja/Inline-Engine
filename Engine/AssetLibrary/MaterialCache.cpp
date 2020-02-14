#include "MaterialCache.hpp"

#include "ImageCache.hpp"
#include "MaterialShaderCache.hpp"

#include <fstream>
#include <rapidjson/document.h>


namespace inl::asset {


struct StringErrorPosition {
	int lineNumber;
	int characterNumber;
	std::string line;
};


static StringErrorPosition GetStringErrorPosition(const std::string& str, size_t errorCharacter) {
	int currentCharacter = 0;
	int characterNumber = 0;
	int lineNumber = 0;
	while (currentCharacter < errorCharacter && currentCharacter < str.size()) {
		if (str[currentCharacter] == '\n') {
			++lineNumber;
			characterNumber = 0;
		}
		++currentCharacter;
		++characterNumber;
	}

	size_t lineBegIdx = str.rfind('\n', errorCharacter);
	size_t lineEndIdx = str.find('\n', errorCharacter);

	if (lineBegIdx = str.npos) {
		lineBegIdx = 0;
	}
	else {
		++lineBegIdx; // we don't want to keep the '\n'
	}
	if (lineEndIdx != str.npos && lineEndIdx > 0) {
		--lineEndIdx; // we don't want to keep the '\n'
	}

	return { lineNumber, characterNumber, str.substr(lineBegIdx, lineEndIdx) };
}


static void AssertThrow(bool condition, const std::string& text) {
	if (!condition) {
		throw InvalidArgumentException(text);
	}
}


MaterialCache::MaterialCache(gxeng::IGraphicsEngine& engine, MaterialShaderCache& shaderCache, ImageCache& imageCache)
	: m_engine(engine), m_shaderCache(shaderCache), m_imageCache(imageCache) {}


std::shared_ptr<gxeng::IMaterial> MaterialCache::Create(const std::filesystem::path& path) {
	std::shared_ptr<gxeng::IMaterial> material(m_engine.CreateMaterial());
	Reload(*material, path);

	return material;
}


void MaterialCache::Reload(gxeng::IMaterial& asset, const std::filesystem::path& path) {
	using namespace rapidjson;

	std::ifstream file(path);
	if (!file.is_open()) {
		throw FileNotFoundException("Asset file exists but cannot be opened.", path.generic_string());
	}
	std::string desc((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Parse document
	Document doc;
	doc.Parse(desc.c_str());
	ParseErrorCode ec = doc.GetParseError();
	if (ec != kParseErrorNone) {
		size_t errorCharacter = doc.GetErrorOffset();
		auto [lineNumber, characterNumber, line] = GetStringErrorPosition(desc, errorCharacter);
		throw InvalidArgumentException("JSON descripion has syntax errors.", "Check line " + std::to_string(lineNumber) + ":" + std::to_string(characterNumber));
	}

	AssertThrow(doc.IsObject(), "Material JSON document must have an object as root.");
	AssertThrow(doc.HasMember("shader") && doc["shader"].IsString(), R"(Material JSON document must have members "shader" and "inputs")");
	AssertThrow(doc.HasMember("inputs"), R"(Material JSON document must have members "header", "shader" and "inputs")");


	std::string shaderName = doc["shader"].GetString();

	std::shared_ptr<gxeng::IMaterialShader> shader = m_shaderCache.Load(shaderName);
	asset.SetShader(shader.get());

	const auto& inputs = doc["inputs"];


	auto SetParam = [this](gxeng::IMaterial::Parameter& param, const std::string& name, const Value& value) {
		try {
			if (value.IsString()) {
				SetMaterialParameter(param, value.GetString());
			}
			else if (value.IsFloat()) {
				SetMaterialParameter(param, value.GetFloat());
			}
			else {
				throw InvalidArgumentException("Material inputs must be either of Image (string of path), Vec4 (string of Vec4) or float (string of float or float)");
			}
		}
		catch (InvalidArgumentException& ex) {
			throw InvalidArgumentException(ex.Message(), std::string("While parsing input \"") + name + "\"");
		}
	};

	if (inputs.IsObject()) {
		for (auto it = inputs.MemberBegin(); it != inputs.MemberEnd(); ++it) {
			std::string name = it->name.GetString();
			gxeng::IMaterial::Parameter& param = asset.GetParameter(name);
			SetParam(param, name, it->value);
		}
	}
	else if (inputs.IsArray()) {
		int idx = 0;
		for (auto it = inputs.Begin(); it != inputs.End(); ++it, ++idx) {
			gxeng::IMaterial::Parameter& param = asset.GetParameter(idx);
			SetParam(param, std::to_string(idx), *it);
		}
	}
	else {
		throw InvalidArgumentException("Material JSON input list must be an object with key-value pairs or an array with the values.");
	}
}


void MaterialCache::SetMaterialParameter(gxeng::IMaterial::Parameter& param, std::string value) {
	switch (param.GetType()) {
		case gxeng::eMaterialShaderParamType::COLOR: {
			const char* endptr;
			Vec4 parsedValue = strtovec<Vec4>(value.c_str(), &endptr);
			if (endptr == value.c_str()) {
				throw InvalidArgumentException("Given value could not be converted to Vec4.");
			}
			param = parsedValue;
			break;
		}
		case gxeng::eMaterialShaderParamType::VALUE: {
			char* endptr;
			float parsedValue = std::strtof(value.c_str(), &endptr);
			if (endptr == value.c_str()) {
				throw InvalidArgumentException("Given value could not be converted to float.");
			}
			param = parsedValue;
			break;
		}
		case gxeng::eMaterialShaderParamType::BITMAP_COLOR_2D: [[fallthrough]];
		case gxeng::eMaterialShaderParamType::BITMAP_VALUE_2D: {
			auto image = m_imageCache.Load(value);
			param = image.get();
			break;
		}
		default: throw InvalidArgumentException("Parameter and given value have different types.");
	}
}


void MaterialCache::SetMaterialParameter(gxeng::IMaterial::Parameter& param, float value) {
	if (param.GetType() == gxeng::eMaterialShaderParamType::VALUE) {
		param = value;
	}
	else {
		throw InvalidArgumentException("Parameter is not a float.");
	}
}


} // namespace inl::asset
