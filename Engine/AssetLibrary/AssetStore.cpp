#include "AssetStore.hpp"
#include "Model.hpp"
#include "Image.hpp"

#include <rapidjson/document.h>
#include <cstdlib>

#include <InlineMath.hpp>


namespace inl::asset {


static void AssertThrow(bool condition, const std::string& text) {
	if (!condition) {
		throw InvalidArgumentException(text);
	}
};
struct StringErrorPosition {
	int lineNumber;
	int characterNumber;
	std::string line;
};
static StringErrorPosition GetStringErrorPosition(const std::string& str, size_t errorCharacter);




AssetStore::AssetStore(gxeng::GraphicsEngine* graphicsEngine, pxeng_bl::PhysicsEngine* physicsEngine) {
	m_graphicsEngine = graphicsEngine;
	m_physicsEngine = physicsEngine;
}


std::shared_ptr<gxeng::Mesh> AssetStore::LoadGraphicsMesh(std::filesystem::path path) {
	auto& cache = m_cachedGraphicsMeshes[path];
	if (!cache.m_forced) {
		cache.m_forced = cache.m_reference.lock();
	}
	if (!cache.m_forced) {
		cache.m_forced = ForceLoadGraphicsMesh(path);
		cache.m_reference = cache.m_forced;
	}
	return cache.m_forced;
}


std::shared_ptr<gxeng::Image> AssetStore::LoadImage(std::filesystem::path path) {
	auto& cache = m_cachedImages[path];
	if (!cache.m_forced) {
		cache.m_forced = cache.m_reference.lock();
	}
	if (!cache.m_forced) {
		cache.m_forced = ForceLoadImage(path);
		cache.m_reference = cache.m_forced;
	}
	return cache.m_forced;
}


std::shared_ptr<gxeng::MaterialShader> AssetStore::LoadMaterialShader(std::filesystem::path path) {
	auto& cache = m_cachedMaterialShaders[path];
	if (!cache.m_forced) {
		cache.m_forced = cache.m_reference.lock();
	}
	if (!cache.m_forced) {
		cache.m_forced = ForceLoadMaterialShader(path);
		cache.m_reference = cache.m_forced;
	}
	return cache.m_forced;
}


std::shared_ptr<gxeng::Material> AssetStore::LoadMaterial(std::filesystem::path path) {
	auto& cache = m_cachedMaterials[path];
	if (!cache.m_forced) {
		cache.m_forced = cache.m_reference.lock();
	}
	if (!cache.m_forced) {
		cache.m_forced = ForceLoadMaterial(path);
		cache.m_reference = cache.m_forced;
	}
	return cache.m_forced;
}


std::shared_ptr<pxeng_bl::MeshShape> AssetStore::LoadPhysicsMesh(std::filesystem::path path, bool dynamic) {
	auto& cache = m_cachedPhysicsMeshes[path];
	if (!cache.m_forced) {
		cache.m_forced = cache.m_reference.lock();
	}
	if (!cache.m_forced) {
		cache.m_forced = ForceLoadPhysicsMesh(path, dynamic);
		cache.m_reference = cache.m_forced;
	}
	return cache.m_forced;
}


void AssetStore::AddSourceDirectory(std::filesystem::path directory) {
	m_directories.insert(directory);
}
void AssetStore::RemoveSourceDirectory(std::filesystem::path directory) {
	auto it = m_directories.find(directory);
	if (it == m_directories.end()) {
		throw InvalidArgumentException("Directory is not part of the asset store.");
	}
	m_directories.erase(it);
}
void AssetStore::ClearSourceDirectories() {
	m_directories.clear();
}


std::shared_ptr<gxeng::Mesh> AssetStore::ForceLoadGraphicsMesh(std::filesystem::path path) {
	path = GetFullPath(path);

	Model model{ path.generic_u8string() };

	CoordSysLayout csys;
	csys.x = AxisDir::POS_X;
	csys.y = AxisDir::POS_Z;
	csys.z = AxisDir::NEG_Y;

	auto vertices = model.GetVertices<gxeng::Position<0>, gxeng::Normal<0>, gxeng::TexCoord<0>>(0, csys);
	auto indices = model.GetIndices(0);

	std::shared_ptr<gxeng::Mesh> mesh(m_graphicsEngine->CreateMesh());

	mesh->Set(vertices.data(), &vertices[0].GetReader(), vertices.size(), indices.data(), indices.size());

	return mesh;
}


std::shared_ptr<gxeng::MaterialShader> AssetStore::ForceLoadMaterialShader(std::filesystem::path path) {
	path = GetFullPath(path);

	std::ifstream file(path);
	if (!file.is_open()) {
		throw FileNotFoundException("Asset file exists but cannot be opened.", path.generic_u8string());
	}
	std::string desc((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	std::shared_ptr<gxeng::MaterialShaderGraph> resource(m_graphicsEngine->CreateMaterialShaderGraph());
	resource->SetGraph(desc);

	return resource;
}


std::shared_ptr<gxeng::Material> AssetStore::ForceLoadMaterial(std::filesystem::path path) {
	using namespace rapidjson;

	path = GetFullPath(path);

	std::ifstream file(path);
	if (!file.is_open()) {
		throw FileNotFoundException("Asset file exists but cannot be opened.", path.generic_u8string());
	}
	std::string desc((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Parse document
	Document doc;
	doc.Parse(desc.c_str());
	ParseErrorCode ec = doc.GetParseError();
	if (ec != kParseErrorNone) {
		size_t errorCharacter = doc.GetErrorOffset();
		auto[lineNumber, characterNumber, line] = GetStringErrorPosition(desc, errorCharacter);
		throw InvalidArgumentException("JSON descripion has syntax errors.", "Check line " + std::to_string(lineNumber) + ":" + std::to_string(characterNumber));
	}

	AssertThrow(doc.IsObject(), "Material JSON document must have an object as root.");
	AssertThrow(doc.HasMember("shader") && doc["shader"].IsString(), R"(Material JSON document must have members "shader" and "inputs")");
	AssertThrow(doc.HasMember("inputs"), R"(Material JSON document must have members "header", "shader" and "inputs")");


	std::string shaderName = doc["shader"].GetString();

	std::shared_ptr<gxeng::MaterialShader> shader = LoadMaterialShader(shaderName);
	std::shared_ptr<gxeng::Material> material(m_graphicsEngine->CreateMaterial());
	material->SetShader(shader.get());

	const auto& inputs = doc["inputs"];


	auto SetParam = [this](gxeng::Material::Parameter& param, const std::string& name, const Value& value) {
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
			gxeng::Material::Parameter& param = (*material)[name];
			SetParam(param, name, it->value);
		}
	}
	else if (inputs.IsArray()) {
		int idx;
		for (auto it = inputs.Begin(); it != inputs.End(); ++it, ++idx) {
			gxeng::Material::Parameter& param = (*material)[idx];
			SetParam(param, std::to_string(idx), *it);
		}
	}
	else {
		throw InvalidArgumentException("Material JSON input list must be an object with key-value pairs or an array with the values.");
	}

	return material;
}


std::shared_ptr<pxeng_bl::MeshShape> AssetStore::ForceLoadPhysicsMesh(std::filesystem::path path, bool dynamic) {
	path = GetFullPath(path);

	Model model{ path.generic_u8string() };

	CoordSysLayout csys;
	csys.x = AxisDir::POS_X;
	csys.y = AxisDir::POS_Z;
	csys.z = AxisDir::NEG_Y;

	auto vertices = model.GetVertices<gxeng::Position<0>>(0, csys);
	auto indices = model.GetIndices(0);

	std::shared_ptr<pxeng_bl::MeshShape> mesh(m_physicsEngine->CreateMeshShape());
	
	static_assert(sizeof(vertices[0]) == sizeof(Vec3));
	mesh->SetMesh(reinterpret_cast<const Vec3*>(vertices.data()), vertices.size(), indices.data(), indices.size());

	return mesh;
}


template <gxeng::ePixelChannelType Type>
gxeng::IPixelReader& GetPixelReaderCh(int channelCount) {
	switch (channelCount) {
		case 1: return gxeng::Pixel<Type, 1, gxeng::ePixelClass::LINEAR>::Reader();
		case 2: return gxeng::Pixel<Type, 2, gxeng::ePixelClass::LINEAR>::Reader();
		case 3: return gxeng::Pixel<Type, 3, gxeng::ePixelClass::LINEAR>::Reader();
		case 4: return gxeng::Pixel<Type, 4, gxeng::ePixelClass::LINEAR>::Reader();
		default: throw InvalidArgumentException("Image must have 1-4 channels.");
	}
}


gxeng::IPixelReader& GetPixelReader(eChannelType channelType, int channelCount) {
	switch (channelType) {
		case eChannelType::INT8: return GetPixelReaderCh<gxeng::ePixelChannelType::INT8_NORM>(channelCount);
		case eChannelType::INT16: return GetPixelReaderCh<gxeng::ePixelChannelType::INT16_NORM>(channelCount);
		case eChannelType::INT32: return GetPixelReaderCh<gxeng::ePixelChannelType::INT32>(channelCount);
		case eChannelType::FLOAT: return GetPixelReaderCh<gxeng::ePixelChannelType::FLOAT32>(channelCount);
		default: std::terminate();
	}
}


std::shared_ptr<gxeng::Image> AssetStore::ForceLoadImage(std::filesystem::path path) {
	path = GetFullPath(path);

	Image image{ path.generic_u8string() };
	int channelCount = image.GetChannelCount();
	eChannelType channelType = image.GetType();

	gxeng::IPixelReader& reader = GetPixelReader(channelType, channelCount);
	
	std::shared_ptr<gxeng::Image> resource(m_graphicsEngine->CreateImage());
	resource->SetLayout(image.GetWidth(), (uint32_t)image.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR);
	resource->Update(0, 0, image.GetWidth(), (uint32_t)image.GetHeight(), 0, image.GetData(), reader);

	return resource;
}


void AssetStore::SetMaterialParameter(gxeng::Material::Parameter& param, std::string value) {
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
			auto image = LoadImage(value);
			param = image.get();
			break;
		}
		default: throw InvalidArgumentException("Parameter and given value have different types.");
	}
}


void AssetStore::SetMaterialParameter(gxeng::Material::Parameter& param, float value) {
	if (param.GetType() == gxeng::eMaterialShaderParamType::VALUE) {
		param = value;
	}
	else {
		throw InvalidArgumentException("Parameter is not a float.");
	}
}


std::filesystem::path AssetStore::GetFullPath(std::filesystem::path localPath) const {
	for (auto& directory : m_directories) {
		std::filesystem::path fullPath = directory / localPath;
		bool exists = std::filesystem::exists(fullPath);
		if (exists) {
			return fullPath;
		}
	}
	throw FileNotFoundException("File not found in any of the directories specified for assets files.", localPath.generic_u8string());
}





StringErrorPosition GetStringErrorPosition(const std::string& str, size_t errorCharacter) {
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




} // namespace inl::asset