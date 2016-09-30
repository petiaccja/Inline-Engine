#pragma once

#include "../GraphicsApi_LL/IRootSignature.hpp"
#include "../GraphicsApi_LL/ICommandList.hpp"

#include <cstdint>
#include <cassert>
#include <initializer_list>


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
/// You can bind resources to the register via <see cref="Binder"/> and <see cref="BasicCommandList"/>.
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
	float relativeAccessFrequency; /// <summary> Not used currently. TODO: Read more about this aspect. </summary>
	float relativeChangeFrequency; /// <summary> How often will you change this binding relative to others. Absolute value does not matter. </summary>
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
	/// <summary> Create a binder from specified binding points. </summary>
	Binder(std::initializer_list<BindParameterDesc> parameters);

	/// <summary> Get where in the root signature the specified parameter lies. </summary>
	/// <param name="parameter"> The parameter to query. </param>
	/// <param name="rootParamIndex"> The index of the record in the root signature. </param>
	/// <param name="rootTableIndex"> If the above record is a descriptor table, the index in the table. Otherwise undefined. </param>
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