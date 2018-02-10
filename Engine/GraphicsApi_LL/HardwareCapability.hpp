#pragma once

#include <limits>
#include <vector>

#include <InlineMath.hpp>

#include "Common.hpp"


namespace inl::gxapi {


static const unsigned UNLIMITED = std::numeric_limits<decltype(UNLIMITED)>::max();



//------------------------------------------------------------------------------
// Capability structures
//------------------------------------------------------------------------------


struct CapsResourceBinding {
	unsigned descriptorsPerHeap = 1; // Max number of descriptors in a descriptor heap.
	unsigned cbvsPerStage = 1; // Max number of descriptors bound per shader stage.
	unsigned srvsPerStage = 1; // Max number of descriptors bound per shader stage.
	unsigned uavsPerStage = 1; // Max number of descriptors bound per shader stage.
	unsigned samplersPerStage = 1; // Max number of samplers bound per stage.
	unsigned unboundEntries = 0; // 0 - all root sig entries must be bound, 1 - except SRVs, 2 - nothing needed

	static CapsResourceBinding Dx12Tier1() { return { 1'000'000, 14, 128, 8, 16, 0}; }
	static CapsResourceBinding Dx12Tier2() { return { 1'000'000, 14, UNLIMITED, 64, UNLIMITED, 1}; }
	static CapsResourceBinding Dx12Tier3() { return { UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, 2}; }

	bool operator>=(const CapsResourceBinding& rhs) const {
		return descriptorsPerHeap >= rhs.descriptorsPerHeap
			&& cbvsPerStage >= rhs.cbvsPerStage
			&& srvsPerStage >= rhs.srvsPerStage
			&& uavsPerStage >= rhs.uavsPerStage
			&& samplersPerStage >= rhs.samplersPerStage
			&& unboundEntries >= rhs.unboundEntries;
	}
	bool operator<=(const CapsResourceBinding& rhs) const {
		return rhs >= *this;
	}

	int GetDx12Tier() const {
		if (*this >= Dx12Tier3())
			return 3;
		else if (*this >= Dx12Tier2())
			return 2;
		else if (*this >= Dx12Tier1())
			return 1;
		else
			return 0;
	}
};


struct CapsTiledResources {
	bool tiledBufferAvailable = false;
	bool tiledTexture2DAvailable = false;
	bool sampleWithLodClampAvailable = false;
	bool sampleWithFeedbackAvailable = false;
	bool nullTileReadDefined = false;
	bool nullTileWriteDefined = false;
	bool tiledTexture3DAvailable = false;

	static CapsTiledResources Dx12Tier1() { return { true, true, false, false, false, false, false }; }
	static CapsTiledResources Dx12Tier2() { return { true, true, true, true, true, true, false }; }
	static CapsTiledResources Dx12Tier3() { return { true, true, true, true, true, true, true }; }

	bool operator>=(const CapsTiledResources& rhs) const {
		return tiledBufferAvailable >= rhs.tiledBufferAvailable
			&& tiledTexture2DAvailable >= rhs.tiledTexture2DAvailable
			&& sampleWithLodClampAvailable >= rhs.sampleWithLodClampAvailable
			&& sampleWithFeedbackAvailable >= rhs.sampleWithFeedbackAvailable
			&& nullTileReadDefined >= rhs.nullTileReadDefined
			&& nullTileWriteDefined >= rhs.nullTileWriteDefined
			&& tiledTexture3DAvailable >= rhs.tiledTexture3DAvailable;
	}
	bool operator<=(const CapsTiledResources& rhs) const {
		return rhs >= *this;
	}

	int GetDx12Tier() const {
		if (*this >= Dx12Tier3())
			return 3;
		else if (*this >= Dx12Tier2())
			return 2;
		else if (*this >= Dx12Tier1())
			return 1;
		else
			return 0;
	}
};


struct CapsConservativeRasterization {
	unsigned uncertaintyRegion = 1;
	bool postSnapDegenerateTrianglesCulled = false;
	bool innerInputCoverage = false;

	static CapsConservativeRasterization Dx12Tier1() { return { 2, false, false }; }
	static CapsConservativeRasterization Dx12Tier2() { return { 256, true, false }; }
	static CapsConservativeRasterization Dx12Tier3() { return { 256, true, true }; }

	bool operator>=(const CapsConservativeRasterization& rhs) const {
		return uncertaintyRegion >= rhs.uncertaintyRegion
			&& postSnapDegenerateTrianglesCulled >= rhs.postSnapDegenerateTrianglesCulled
			&& innerInputCoverage >= rhs.innerInputCoverage;
	}
	bool operator<=(const CapsConservativeRasterization& rhs) const {
		return rhs >= *this;
	}

	int GetDx12Tier() const {
		if (*this >= Dx12Tier3())
			return 3;
		else if (*this >= Dx12Tier2())
			return 2;
		else if (*this >= Dx12Tier1())
			return 1;
		else
			return 0;
	}
};


struct CapsResourceHeaps {
	bool mixedHeapsSupported = false; // false: D3D12 Tier 1 heaps, true: Tier 2 heaps

	static CapsResourceHeaps Dx12Tier1() { return { false }; }
	static CapsResourceHeaps Dx12Tier2() { return { true }; }

	bool operator>=(const CapsResourceHeaps& rhs) const {
		return mixedHeapsSupported >= rhs.mixedHeapsSupported;
	}
	bool operator<=(const CapsResourceHeaps& rhs) const {
		return rhs >= *this;
	}

	int GetDx12Tier() const {
		if (*this >= Dx12Tier2())
			return 2;
		else if (*this >= Dx12Tier1())
			return 1;
		else
			return 0;
	}
};


struct CapsAdditional {
	bool rovsSupported = false;
	unsigned shaderModelMajor = 5;
	unsigned shaderModelMinor = 1;
	unsigned virtualAddressBitsPerResource = 32;
	unsigned virtualAddressBitsPerProcess = 32;

	bool operator>=(const CapsAdditional& rhs) const {
		int shm = shaderModelMajor*100 + shaderModelMinor;
		int shmrhs = rhs.shaderModelMajor*100 + rhs.shaderModelMinor;
		return rovsSupported >= rhs.rovsSupported
			&& shm >= shmrhs
			&& virtualAddressBitsPerResource >= rhs.virtualAddressBitsPerResource
			&& virtualAddressBitsPerProcess >= rhs.virtualAddressBitsPerProcess;
	}
	bool operator<=(const CapsAdditional& rhs) const {
		return rhs >= *this;
	}
};


struct CapsLimits {
	uint64_t texture1DSize = 1;
	Vec2u64 texture2DSize = { 1, 1 };
	Vec3u64 texture3DSize = { 1, 1, 1 };
	unsigned textureRepeat = 1;
	unsigned anisotropy = 1;
	uint64_t primitiveCount = 1;
	uint64_t vertexCount = 1;
	unsigned inputSlots = 1;
	unsigned multipleRenderTargets = 1;

	bool operator>=(const CapsLimits& rhs) const {
		return texture1DSize >= rhs.texture1DSize
			&& texture2DSize.x >= rhs.texture2DSize.x && texture2DSize.y >= rhs.texture2DSize.y
			&& texture3DSize.x >= rhs.texture3DSize.x && texture3DSize.y >= rhs.texture3DSize.y && texture3DSize.z >= rhs.texture3DSize.z
			&& textureRepeat >= rhs.textureRepeat
			&& anisotropy >= rhs.anisotropy
			&& primitiveCount >= rhs.primitiveCount
			&& vertexCount >= rhs.vertexCount
			&& inputSlots >= rhs.inputSlots
			&& multipleRenderTargets >= rhs.multipleRenderTargets;
	}
	bool operator<=(const CapsLimits& rhs) const {
		return rhs >= *this;
	}
};



//------------------------------------------------------------------------------
// Capability enums
//------------------------------------------------------------------------------

struct eCapsFormatUsage_Base {
	enum EnumT : uint64_t {
		// Type
		BUFFER,
		VERTEX_BUFFER,
		SO_BUFFER,
		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE,
		// Usage
		SAMPLE,
		SAMPLE_LINEAR,
		RENDER_TARGET,
		RENDER_TARGET_BLEND,
		DEPTH_STENCIL,
		UNORDERED_ACCESS_LOAD,
		UNORDERED_ACCESS_STORE,
		UNORDERED_ACCESS_ATOMIC
	};
};

using eCapsFormatUsage = EnumFlag_Helper<eCapsFormatUsage_Base>;



//------------------------------------------------------------------------------
// Decision helper
//------------------------------------------------------------------------------

struct CapsRequirementSet {
	CapsResourceBinding resourceBinding;
	CapsTiledResources tiledResources;
	CapsConservativeRasterization conservativeRasterization;
	CapsResourceHeaps resourceHeaps;
	CapsAdditional additional;
	CapsLimits limits;
	std::vector<std::pair<eFormat, eCapsFormatUsage>> formats;
};



//------------------------------------------------------------------------------
// Query interface
//------------------------------------------------------------------------------


class ICapabilityQuery {
public:
	virtual ~ICapabilityQuery() = default;

	virtual CapsResourceBinding QueryResourceBinding() const = 0;
	virtual CapsTiledResources QueryTiledResources() const = 0;
	virtual CapsConservativeRasterization QueryConservativeRasterization() const = 0;
	virtual CapsResourceHeaps QueryResourceHeaps() const = 0;
	virtual CapsAdditional QueryAdditional() const = 0;
	virtual CapsLimits QueryLimits() const = 0;
	virtual eCapsFormatUsage QueryFormat(eFormat format) const = 0;

	virtual bool SupportsAll(const CapsRequirementSet& requiredFeatures) const = 0;
};


} // namespace inl::gxapi