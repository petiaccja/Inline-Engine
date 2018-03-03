#pragma once

#include "../GraphicsApi_LL/IRootSignature.hpp"
#include "../GraphicsApi_LL/ICommandList.hpp"

#include <cstdint>
#include <cassert>
#include <iostream>
#include <initializer_list>


namespace inl {namespace gxapi {
	class IGraphicsApi;
}
}


namespace inl {namespace gxeng {
class Binder;
}
}
inline std::ostream& operator<<(std::ostream& os, const inl::gxeng::Binder& binder);


namespace inl {
namespace gxeng {


/// <summary> Specifies the target registers type. </summary>
enum class eBindParameterType {
	CONSTANT = 2,
	TEXTURE = 3,
	UNORDERED = 4,
	SAMPLER = 5,
};


/// <summary>
/// Bind parameters identify a register in the shader. 
/// You can bind resources to the register via <see cref="Binder"/> and <see cref="inl::gxeng::BasicCommandList"/>.
/// </summary>
struct BindParameter {
	constexpr BindParameter(eBindParameterType type = eBindParameterType::CONSTANT, unsigned reg = 0, unsigned space = 0) :
		type(type), reg(reg), space(space) {}
	eBindParameterType type;
	unsigned reg; // register
	unsigned space; // register space

	bool operator==(const BindParameter& rhs) {
		return type == rhs.type && reg == rhs.reg && space == rhs.space;
	}
	bool operator!=(const BindParameter& rhs) {
		return !(*this == rhs);
	}
};


/// <summary>
/// Used to construct a Binder object.
/// You can (and have to) specify other things besides the register.
/// </summary>
struct BindParameterDesc {
	BindParameter parameter; /// <summary> Target register. </summary>
	unsigned constantSize = 0; /// <summary> Size of constant in bytes. Set to zero if unknown. </summary>
	float relativeAccessFrequency = 1; /// <summary> Not used currently. TODO: Read more about this aspect. </summary>
	float relativeChangeFrequency = 1; /// <summary> How often will you change this binding relative to others. Absolute value does not matter. </summary>
	gxapi::eShaderVisiblity shaderVisibility = gxapi::eShaderVisiblity::ALL;
};


/// <summary>
/// A binder describes the link between resources and shader registers.
/// </summary>
/// <remarks>
/// The binder is a flat structure, that is, a mapping between shader registers and resources.
/// Hides the complexity of Root Signatures, and optimizes parameter layout for preformance and space.
/// </remarks>
class Binder {
	friend std::ostream& ::operator<<(std::ostream& os, const Binder& binder);
private:
	// Specifies which BindParameter corresponds to Root Signature's parameters.
	struct RootParameterMapping {
		BindParameter bindParam; // parameter description
		int constantCount = 0; // number of 32 bit constants, 0 if not an inline constant
		int rootParamIndex = -1; // which root signature parameter it is in
		int rootTableIndex = -1; // if the root parameter is a descriptor table, specifies the index within the table, -1 if not a table
	};

	// Radix sort for BindParameters
	static bool RadixLess(const BindParameter& lhs, const BindParameter& rhs);
public:
	Binder() = default;

	/// <summary> Create a binder from specified binding points. </summary>
	Binder(gxapi::IGraphicsApi* gxApi, const std::vector<BindParameterDesc>& parameters, const std::vector<gxapi::StaticSamplerDesc>& staticSamplers = {});

	/// <summary> Get where in the root signature the specified parameter lies. </summary>
	/// <param name="parameter"> The parameter to query. </param>
	/// <param name="rootParamIndex"> The index of the record in the root signature. </param>
	/// <param name="rootTableIndex"> If the above record is a descriptor table, the index in the table. Otherwise undefined. </param>
	void Translate(BindParameter parameter, int& rootParamIndex, int& rootTableIndex) const;

	/// <summary> Return the underlying root signature object. </summary>
	gxapi::IRootSignature* GetRootSignature() const { return m_rootSignature.get(); }

	/// <summary> Return the description of the underying root signature object. </summary>
	/// <remarks> Use this to determine the type of slots returned by <see cref="Translate"/>. </remarks>
	const gxapi::RootSignatureDesc& GetRootSignatureDesc() const { return m_rootSignatureDesc; }

	/// <summary> True if the binder has been initialized with a list of parameters. </summary>
	operator bool() const { return (bool)m_rootSignature; }
private:
	void CalculateLayout(const std::vector<BindParameterDesc>& parameters);
	void DistributeParameters(const std::vector<BindParameterDesc>& parameters,
		std::vector<std::vector<BindParameterDesc>> & tableParams,
		std::vector<BindParameterDesc> & samplerParams,
		std::vector<BindParameterDesc> & constantParams);
	gxapi::DescriptorRange::eType CastRangeType(eBindParameterType source);

	std::pair<std::vector<RootParameterMapping>::const_iterator, bool> FindMapping(BindParameter param) const;
private:
	std::vector<RootParameterMapping> m_parameters;
	std::unique_ptr<gxapi::IRootSignature> m_rootSignature;
	gxapi::RootSignatureDesc m_rootSignatureDesc;

	// Maximum root signature size = 64 DWORDs.
	// TODO: query from gxapi!
	static constexpr int maxSize = 64*sizeof(uint32_t); 
};



} // namespace gxeng
} // namespace inl


#include <map>
#include <list>
inline std::ostream& ::operator<<(std::ostream& os, const inl::gxeng::Binder& binder) {
	struct RootSignatureSlot {
		bool isTable;
		std::list<inl::gxeng::Binder::RootParameterMapping> bindParameters;
	};
	std::map<int, RootSignatureSlot> slots;
	for (auto& v : binder.m_parameters) {
		auto& slot = slots[v.rootParamIndex];
		slot.isTable = v.rootTableIndex >= 0;
		slot.bindParameters.push_back(v);
	}

	for (auto it = slots.begin(); it != slots.end(); ++it) {
		int index = it->first;
		auto& desc = it->second;

		if (desc.isTable) {
			os << "[";

			for (auto it = desc.bindParameters.begin(); it != desc.bindParameters.end(); ++it) {
				os << [](inl::gxeng::eBindParameterType type)
				{
					switch (type) {
					case inl::gxeng::eBindParameterType::CONSTANT: return "C";
					case inl::gxeng::eBindParameterType::TEXTURE: return "T";
					case inl::gxeng::eBindParameterType::UNORDERED: return "U";
					case inl::gxeng::eBindParameterType::SAMPLER: return "S";
					default: assert(false);
					}
				}(it->bindParam.type);
				if (++decltype(it)(it) != desc.bindParameters.end()) {
					os << ", ";
				}
			}

			os << "]";
		}
		else {
			os << [](inl::gxeng::eBindParameterType type)
			{
				switch (type) {
				case inl::gxeng::eBindParameterType::CONSTANT: return "C";
				case inl::gxeng::eBindParameterType::TEXTURE: return "T";
				case inl::gxeng::eBindParameterType::UNORDERED: return "U";
				case inl::gxeng::eBindParameterType::SAMPLER: return "S";
				default: assert(false);
				}
			}(desc.bindParameters.begin()->bindParam.type);
			if (desc.bindParameters.begin()->bindParam.type == inl::gxeng::eBindParameterType::CONSTANT && desc.bindParameters.begin()->constantCount > 0) {
				os << "...";
			}
		}

		if (++decltype(it)(it) != slots.end()) {
			os << " | ";
		}
	}

	return os;
}