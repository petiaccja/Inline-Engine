#pragma once

#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>

#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsApi_LL/Common.hpp>


namespace inl {
namespace gxeng {


/// <summary> Contains the compiled binaries for a shader stage (VS, PS, etc.). </summary>
class ShaderStage {
public:
	ShaderStage() = default;
	ShaderStage(std::vector<uint8_t> binary) : m_binary(std::move(binary)) {}

	operator bool() const { return m_binary.size() > 0; }
	operator gxapi::ShaderByteCodeDesc() const { return gxapi::ShaderByteCodeDesc(m_binary.data(), m_binary.size()); }

	const void* Data() const { return m_binary.data(); }
	size_t Size() const { return m_binary.size(); }
private:
	std::vector<uint8_t> m_binary;
};

#ifdef _MSC_VER
#pragma warning(disable: 4522)
#endif

/// <summary> Helper class for keeping track of which shader stages have been compiled. </summary>
struct ShaderParts {
	bool vs = false, hs = false, ds = false, gs = false, ps = false, cs = false;
	bool SubsetOf(const volatile ShaderParts& other) const volatile {
		return (vs <= other.vs)
			&& (hs <= other.hs)
			&& (ds <= other.ds)
			&& (gs <= other.gs)
			&& (ps <= other.ps)
			&& (cs <= other.cs);
	}
	ShaderParts SetSubtract(const volatile ShaderParts& other) const volatile {
		ShaderParts ret;
		ret.vs = !other.vs && vs;
		ret.hs = !other.hs && hs;
		ret.ds = !other.ds && ds;
		ret.gs = !other.gs && gs;
		ret.ps = !other.ps && ps;
		ret.cs = !other.cs && cs;
		return ret;
	}
	ShaderParts SetUnion(const volatile ShaderParts& other) const volatile {
		ShaderParts ret;
		ret.vs = other.vs || vs;
		ret.hs = other.hs || hs;
		ret.ds = other.ds || ds;
		ret.gs = other.gs || gs;
		ret.ps = other.ps || ps;
		ret.cs = other.cs || cs;
		return ret;
	}
	ShaderParts& operator=(const ShaderParts& other) {
		vs = other.vs;
		hs = other.hs;
		ds = other.ds;
		gs = other.gs;
		ps = other.ps;
		cs = other.cs;
		return *this;
	}
	volatile ShaderParts& operator=(const ShaderParts& other) volatile {
		vs = other.vs;
		hs = other.hs;
		ds = other.ds;
		gs = other.gs;
		ps = other.ps;
		cs = other.cs;
		return *this;
	}
};

#ifdef _MSC_VER
#pragma warning(default: 4522)
#endif

/// <summary> Contains all binaries associated with a shader program.
/// It should be added to a PSO and used in the pipeline. </summary>
class ShaderProgram {
public:
	bool IsCompute() const;
	ShaderStage vs, gs, ds, hs, ps, cs;
};


/// <summary>
/// All shader creation should go through the shader manager. The shader manager
/// keeps a list of directories, executable (PE/ELF) resources and on-line generated
/// shader codes. Shader binaries are requested by name. If the source code associated
/// with the requested name is found either as a file, a resource or memory-string,
/// the code is compiled and the binary is returned.
/// </summary>
/// <remarks>
/// This class is designed for concurrent requests for multiple shaders, so it is
/// partially thread-safe.
/// </remarks>
class ShaderManager {
private:
	struct ShaderId {
		ShaderId() = default;
		ShaderId(std::string name) : name(name) {}
		ShaderId(std::string name, std::string macros) : name(name), macros(macros) {}
		std::string name;
		std::string macros;
		bool operator==(const ShaderId& rhs) const { return name == rhs.name && macros == rhs.macros; }
	};
	struct ShaderStore {
		ShaderProgram program;
		volatile ShaderParts parts;
	};
	struct PathHash {
		size_t operator()(const std::filesystem::path& obj) const {
			return std::filesystem::hash_value(obj);
		}
	};
	struct ShaderIdHash {
		size_t operator()(const ShaderId& obj) const {
			std::hash<std::string> sh;
			return sh(obj.name) ^ sh(obj.macros);
		}
	};

	using PathContainer = std::unordered_set<std::filesystem::path, PathHash>;
	using CodeContainer = std::unordered_map<std::string, std::string>;
	using ShaderContainer = std::unordered_map<ShaderId, std::unique_ptr<ShaderStore>, ShaderIdHash>;
public:
	ShaderManager(gxapi::IGxapiManager* gxapiManager);
	~ShaderManager();

	/// <summary> Adds a new source directory to look for shader codes. </summary> 
	/// <remarks> This method is thread-safe. </remarks>
	void AddSourceDirectory(std::filesystem::path directory);

	/// <summary> Removes a shader source directory from the list. </summary> 
	/// <remarks> This method is thread-safe. </remarks>
	void RemoveSourceDirectory(std::filesystem::path directory);

	/// <summary> Removes all added source directories. </summary>
	void ClearSourceDirectories();

	/// <summary> Query source directories currently in use. </summary> 
	/// <returns> Iterator to the beginning and end of the range containing shader directories. </returns>
	/// <remarks> This method is NOT thread-safe,
	///	adding or removing directories concurrently with this is disallowed. </remarks>
	std::pair<PathContainer::const_iterator, PathContainer::const_iterator> GetSourceDirectories() const;


	/// <summary> Adds a source code to the list. </summary> 
	/// <remarks> This method is thread-safe. </remarks>
	void AddSourceCode(std::string name, std::string sourceCode);

	/// <summary> Removes a source code to the list. </summary> 
	/// <remarks> This method is thread-safe. </remarks>
	void RemoveSourceCode(const std::string& name);

	/// <summary> Removes all added source codes. </summary>
	void ClearSourceCodes();

	/// <summary> Query source codes currently in use. </summary>
	/// <returns> Iterator to the beginning and end of the range containing source codes. </returns>
	std::pair<CodeContainer::const_iterator, CodeContainer::const_iterator> GetSourceCodes() const;


	/// <summary> Set the global flags to compile all shaders. 
	///	Specify debugging, matrix memory layout and optimization here. </summary>
	/// <remarks> This method is NOT thread-safe (though it shouldn't cause much trouble). </remarks>
	void SetShaderCompileFlags(gxapi::eShaderCompileFlags flags);

	/// <summary> Get the shader compilation global flags. </summary>
	/// <remarks> This method is NOT thread-safe (you might read garbage, but won't crash or anything). </remarks>
	gxapi::eShaderCompileFlags GetShaderCompileFlags() const;


	/// <summary> Compile a shader from source. </summary>
	/// <param name="name"> Name of the shader (tipically file name), without extension. </param>
	/// <param name="parts"> Which shader stages should be compiled. </param>
	/// <param name="macros"> A string containing the marco definitions to use for shader compilation. </param>
	const ShaderProgram& CreateShader(const std::string& name, ShaderParts parts, const std::string& macros = {});


	/// <summary> It does not do anything, but I guess it will be good for something in the future. </summary>
	void ReloadShaders();

	/// <summary> Return the source code of a certain shader. </summary>
	std::string LoadShaderSource(const std::string& name) const;

	/// <summary> Compile arbitrary source code without adding it to the library. </summary>
	/// <remarks> Include directives will still work if registered files are referenced. </remarks>
	ShaderProgram CompileShader(const std::string& sourceCode, ShaderParts parts, const std::string& macros = {});
private:
	/// <summary> Find a source in dirs, resource and codes by its name. Does not lock anything. </summary>
	/// <returns> 
	///		A {name, code} pair, where name is the canonical, unique name for the shader,
	///		and code is the actual HLSL(/other) shader code. 
	/// </returns>
	std::pair<std::string, std::string> FindShaderCode(const std::string& name) const;

	/// <summary> Compiles a shader to binary according to parameters. </summary>
	ShaderProgram CompileShaderInternal(const std::string& sourceCode, ShaderParts parts, const std::string& macros);

	// Cuts off extension (only .hlsl, .glsl, .cg, .txt), converts to lowercase.
	static std::string StripShaderName(std::string name, bool lowerCase = true);
private:
	gxapi::IGxapiManager* m_gxapiManager;

	PathContainer m_directories; /// <summary> List of directories where shaders should be searched for. </summary>
	CodeContainer m_codes; /// <summary> List of {fileName, sourceCode} of runtime-added char* shaders. </summary>

	ShaderContainer m_shaders; /// <summary> List of compiled shaders. </summary>

	mutable std::shared_mutex m_sourceMutex; /// <summary> Lock when accessing directory or code maps. </summary>
	std::mutex m_shaderMutex; /// <summary> Shared: when reading m_shaders; Exclusive: when writing m_shaders. </summary>
	std::unique_ptr<std::mutex[]> m_compileMutexes; /// <summary> Hash-modulo select one, lock when accessing hashed shader binary. </summary>
	size_t m_numCompileMutexes;

	gxapi::eShaderCompileFlags m_compileFlags;
};



} // namespace gxeng
} // namespace inl
