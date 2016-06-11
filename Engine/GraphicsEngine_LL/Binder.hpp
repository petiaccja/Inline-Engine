#pragma once

#include "../GraphicsApi_LL/IRootSignature.hpp"
#include "../GraphicsApi_LL/ICommandList.hpp"

#include <cstdint>
#include <cassert>
#include <initializer_list>


namespace inl {
namespace gxeng {



enum class eBindParameterType {
	INLINE_CONSTANT = 1,
	CONSTANT = 2,
	TEXTURE = 3,
	UNORDERED = 4,
	SAMPLER = 5,
	UNKNOWN = 20,
};



struct BindParameter {
	constexpr BindParameter(eBindParameterType type = eBindParameterType::UNKNOWN, unsigned index = 0, unsigned space = 0) :
		type(type), index(index), space(space) {}
	eBindParameterType type;
	unsigned index;
	unsigned space;

	bool operator==(const BindParameter& rhs) {
		return type == rhs.type && index == rhs.index && space == rhs.space;
	}
	bool operator!=(const BindParameter& rhs) {
		return *this != rhs;
	}
};



struct BindParameterDesc {
	BindParameter parameter;
	float relativeAccessFrequency;
	float relativeChangeFrequency;
	unsigned constantSize; /// <summary> Size of inline constant in bytes. </summary>
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
		BindParameter bindParam;
		int rootParamIndex;
		int rootTableIndex;
	};

public:
	Binder(std::initializer_list<BindParameterDesc> parameters);

	void Translate(BindParameter parameter, int& rootParamIndex, int& rootTableIndex);
private:
	std::vector<RootParameterMapping> m_parameters;
	inl::gxapi::IRootSignature* m_rootSignature;
};




} // namespace gxeng
} // namespace inl