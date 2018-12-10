#include "ShaderManager.hpp"

#include <thread>
#include <algorithm>
#include <fstream>
#include <functional>


namespace inl {
namespace gxeng {


ShaderManager::ShaderManager(gxapi::IGxapiManager* gxapiManager)
	: m_gxapiManager(gxapiManager)
{
	unsigned numCores = std::thread::hardware_concurrency();
	numCores = std::max(1u, numCores); // must be at least one core
	numCores = std::min(64u, numCores); // there should not be more than 64 cores... too many mutexes
	m_numCompileMutexes = numCores * 5; // should find some prime larger than X, but that's it for now
	m_compileMutexes = std::make_unique<std::mutex[]>(m_numCompileMutexes);
}

ShaderManager::~ShaderManager() {
	// empty
}


void ShaderManager::AddSourceDirectory(std::filesystem::path directory) {
	std::unique_lock<std::shared_mutex> lkg(m_sourceMutex);

	m_directories.insert(directory);
}

void ShaderManager::RemoveSourceDirectory(std::filesystem::path directory) {
	std::unique_lock<std::shared_mutex> lkg(m_sourceMutex);

	m_directories.erase(directory);
}

void ShaderManager::ClearSourceDirectories() {
	std::unique_lock<std::shared_mutex> lkg(m_sourceMutex);

	m_directories.clear();
}

auto ShaderManager::GetSourceDirectories() const -> std::pair<PathContainer::const_iterator, PathContainer::const_iterator> {
	return{ m_directories.begin(), m_directories.end() };
}


void ShaderManager::AddSourceCode(std::string name, std::string sourceCode) {
	std::unique_lock<std::shared_mutex> lkg(m_sourceMutex);

	m_codes.insert({ name, sourceCode });
}

void ShaderManager::RemoveSourceCode(const std::string& name) {
	std::unique_lock<std::shared_mutex> lkg(m_sourceMutex);

	m_codes.erase(name);
}

void ShaderManager::ClearSourceCodes() {
	std::unique_lock<std::shared_mutex> lkg(m_sourceMutex);

	m_codes.clear();
}

auto ShaderManager::GetSourceCodes() const ->std::pair<CodeContainer::const_iterator, CodeContainer::const_iterator> {
	return{ m_codes.begin(), m_codes.end() };
}


const ShaderProgram& ShaderManager::CreateShader(const std::string& name, ShaderParts requestedParts, const std::string& macros) {
	// lock shader maps
	std::unique_lock<std::mutex> shaderMapLock(m_shaderMutex);

	ShaderId shaderId{ name, macros };
	auto it = m_shaders.find(shaderId);

	// shader exists
	if (it != m_shaders.end()) {
		// check if it has the requested binaries, and return it
		if (requestedParts.SubsetOf(it->second->parts)) {
			return it->second->program;
		}
	}
	// shader does not exist
	else {
		// insert new entry for shader
		auto ins = m_shaders.insert({ name, std::make_unique<ShaderStore>() });
		it = ins.first;
	}
	ShaderStore* shader = it->second.get();

	// release shader maps
	shaderMapLock.unlock();


	// lock source maps (read only lock)
	std::shared_lock<std::shared_mutex> sourceLock(m_sourceMutex);
	// lock the shader itself
	size_t nameHash = ShaderIdHash()(shaderId);
	std::unique_lock<std::mutex> shaderLock(m_compileMutexes[nameHash % m_numCompileMutexes]);

	// find requested shader code
	std::string shaderSource;
	std::string shaderPath;
	{
		auto pathSourcePair = FindShaderCode(name.c_str());
		shaderPath = pathSourcePair.first;
		shaderSource = pathSourcePair.second;
	}

	// determine what parts to compile, and compile them
	ShaderParts partsToCompile = requestedParts.SetSubtract(shader->parts);
	ShaderProgram program;
	try {
		program = CompileShaderInternal(shaderSource.c_str(), partsToCompile, macros.c_str());
	}
	catch (gxapi::ShaderCompilationError& ex) {
		throw gxapi::ShaderCompilationError("Error while compiling shader \"" + shaderPath + "\"", ex.Subject());
	}

	shader->parts = shader->parts.SetUnion(partsToCompile);
	if (partsToCompile.vs) { shader->program.vs = std::move(program.vs); }
	if (partsToCompile.hs) { shader->program.hs = std::move(program.hs); }
	if (partsToCompile.ds) { shader->program.ds = std::move(program.ds); }
	if (partsToCompile.gs) { shader->program.gs = std::move(program.gs); }
	if (partsToCompile.ps) { shader->program.ps = std::move(program.ps); }
	if (partsToCompile.cs) { shader->program.cs = std::move(program.cs); }

	return shader->program;
}


void ShaderManager::SetShaderCompileFlags(gxapi::eShaderCompileFlags flags) {
	m_compileFlags = flags;
}

gxapi::eShaderCompileFlags ShaderManager::GetShaderCompileFlags() const {
	return m_compileFlags;
}

void ShaderManager::ReloadShaders() {
	return;
}


std::string ShaderManager::LoadShaderSource(const std::string& name) const {
	std::shared_lock<std::shared_mutex> lkg(m_sourceMutex);

	auto code = FindShaderCode(name);

	return code.second;
}


ShaderProgram ShaderManager::CompileShader(const std::string& sourceCode, ShaderParts parts, const std::string& macros) {
	// lock source maps (read only lock)
	std::shared_lock<std::shared_mutex> sourceLock(m_sourceMutex);

	return CompileShaderInternal(sourceCode, parts, macros);
}


std::pair<std::string, std::string> ShaderManager::FindShaderCode(const std::string& name) const {
	std::string keyName = StripShaderName(name);
	std::string fileName = StripShaderName(name, false);

	// try it in direct source cache
	auto codeIt = m_codes.find(keyName);
	if (codeIt != m_codes.end()) {
		return { keyName, codeIt->second };
	}

	// try it in directories
	for (const auto& directory : m_directories) {
		auto filepath = directory / (fileName + ".hlsl");
		std::ifstream fs(filepath.c_str());
		if (fs.is_open()) {
			fs.seekg(0, std::ios::end);
			size_t s = fs.tellg();
			fs.seekg(0, std::ios::beg);
			std::unique_ptr<char[]> content = std::make_unique<char[]>(s + 1);
			fs.read(content.get(), s);
			content[s] = '\0';
			return { filepath.generic_string(), content.get() };
		}
	}

	throw FileNotFoundException("Shader was not found.", keyName + "(" + name + " as requested)");
}

ShaderProgram ShaderManager::CompileShaderInternal(const std::string& sourceCode, ShaderParts parts, const std::string& macros) {
	class IncludeProvider : public gxapi::IShaderIncludeProvider {
	public:
		IncludeProvider(std::function<std::string(const char*)> findShader) : m_findShader(findShader) {}
		std::string LoadInclude(const char* includeName, bool systemInclude) override {
			return m_findShader(includeName);
		}
	private:
		std::function<std::string(const char*)> m_findShader;
	};

	IncludeProvider includeProvider([this](const char* name) { return FindShaderCode(name).second; });
	ShaderProgram ret;

	static const char* const mainNames[] = {
		"VSMain",
		"HSMain",
		"DSMain",
		"GSMain",
		"PSMain",
		"CSMain",
	};
	static const gxapi::eShaderType types[] = {
		gxapi::eShaderType::VERTEX,
		gxapi::eShaderType::HULL,
		gxapi::eShaderType::DOMAIN,
		gxapi::eShaderType::GEOMETRY,
		gxapi::eShaderType::PIXEL,
		gxapi::eShaderType::COMPUTE,
	};

	int compileIndices[7]; // last item is a guard (-1), not used otherwise
	{
		for (auto& v : compileIndices) { v = -1; }
		int idx = 0;
		if (parts.vs) { compileIndices[idx] = 0; ++idx; }
		if (parts.hs) { compileIndices[idx] = 1; ++idx; }
		if (parts.ds) { compileIndices[idx] = 2; ++idx; }
		if (parts.gs) { compileIndices[idx] = 3; ++idx; }
		if (parts.ps) { compileIndices[idx] = 4; ++idx; }
		if (parts.cs) { compileIndices[idx] = 5; ++idx; }
	}

	int idx = 0;
	while (compileIndices[idx] != -1) {
		const int stageId = compileIndices[idx];
		const char* mainName = mainNames[stageId];
		gxapi::eShaderType type = types[stageId];
		auto binary = m_gxapiManager->CompileShader(sourceCode.c_str(),
			mainName,
			type,
			m_compileFlags,
			&includeProvider,
			macros.c_str());

		ShaderStage* dest = nullptr;
		switch (type) {
		case gxapi::eShaderType::VERTEX: dest = &ret.vs; break;
			case gxapi::eShaderType::HULL: dest = &ret.hs; break;
			case gxapi::eShaderType::DOMAIN: dest = &ret.ds; break;
			case gxapi::eShaderType::GEOMETRY: dest = &ret.gs; break;
			case gxapi::eShaderType::PIXEL: dest = &ret.ps; break;
			case gxapi::eShaderType::COMPUTE: dest = &ret.cs; break;
		}
		*dest = ShaderStage(std::move(binary.data));
		++idx;
	}

	return ret;
}


std::string ShaderManager::StripShaderName(std::string name, bool lowerCase) {
	// remove extension from the end, if any
	size_t extDot = name.find_last_of('.');
	if (extDot != name.npos) {
		std::string extension = name.substr(extDot + 1);
		if (extension == "txt" || extension == "hlsl" || extension == "glsl" || extension == "cg" || extension == "") {
			name = name.substr(0, extDot);
		}
	}

	// convert to lowercase
	if (lowerCase) {
		for (auto& c : name) {
			c = tolower(c);
		}
	}

	return name;
}


} // namespace gxeng
} // namespace inl