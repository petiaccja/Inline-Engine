#pragma once

#include "../GraphicsApi_LL/IRootSignature.hpp"
#include "../GraphicsApi_LL/ICommandList.hpp"

#include <cstdint>
#include <cassert>
#include <initializer_list>


namespace inl {
namespace gxeng {



enum class eBindParameterType {
	CONSTANT = 2,
	TEXTURE = 3,
	UNORDERED = 4,
	SAMPLER = 5,
};



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
		return *this != rhs;
	}
};



struct BindParameterDesc {
	BindParameter parameter;
	float relativeAccessFrequency;
	float relativeChangeFrequency;
	unsigned constantSize; /// <summary> Size of constant in bytes. Set to zero if unknown. </summary>
};


/// <summary>
/// A binder describes the link between resources and shader registers.
/// </summary>
/// <remarks>
/// The binder is a flat structure, that is, a mapping between shader registers and resources.
/// Hides the complexity of Root Signatures, and optimizes parameter layout for preformance and space.
/// </remarks>
class Binder {
private:
	// Specifies which BindParameter corresponds to Root Signature's parameters.
	struct RootParameterMapping {
		BindParameter bindParam; // parameter description
		int rootParamIndex = -1; // which root signature parameter it is in
		int rootTableIndex = -1; // if the root parameter is a descriptor table, specifies the index within the table
		int constantCount; // number of 32 bit constants
	};

	// Radix sort for BindParameters
	static bool RadixLess(const BindParameter& lhs, const BindParameter& rhs);
public:
	Binder(std::initializer_list<BindParameterDesc> parameters);

	void Translate(BindParameter parameter, int& rootParamIndex, int& rootTableIndex) const;
private:
	void CalculateLayout(const std::initializer_list<BindParameterDesc>& parameters);
	void DistributeParameters(const std::initializer_list<BindParameterDesc>& parameters,
							  std::vector<std::vector<BindParameterDesc>> & tableParams,
							  std::vector<BindParameterDesc> & samplerParams,
							  std::vector<BindParameterDesc> & constantParams);
	gxapi::DescriptorRange::eType CastRangeType(eBindParameterType source);

	std::pair<std::vector<RootParameterMapping>::const_iterator, bool> FindMapping(BindParameter param) const;
private:
	std::vector<RootParameterMapping> m_parameters;
	gxapi::IRootSignature* m_rootSignature;
	static constexpr int maxSize = 64; // maximum root signature size. TODO: query from gxapi!!!
};




} // namespace gxeng
} // namespace inl