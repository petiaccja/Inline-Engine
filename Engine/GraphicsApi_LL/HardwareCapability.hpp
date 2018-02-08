#pragma once
#include <limits>

namespace inl::gxapi {


static const unsigned UNLIMITED = std::numeric_limits<decltype(UNLIMITED)>::max();


struct CapsResourceBinding {
	unsigned descriptorsPerHeap; // Max number of descriptors in a descriptor heap.
	unsigned cbvsPerStage; // Max number of descriptors bound per shader stage.
	unsigned srvsPerStage; // Max number of descriptors bound per shader stage.
	unsigned uavsPerStage; // Max number of descriptors bound per shader stage.
	unsigned samplersPerStage; // Max number of samplers bound per stage.
	unsigned unboundEntries; // 0 - all root sig entries must be bound, 1 - except SRVs, 2 - nothing needed
};


struct CapsTiledResources {
	bool tiledBufferAvailable;
	bool tiledTexture2DAvailable;
	bool sampleWithLodClampAvailable;
	bool sampleWithFeedbackAvailable;
	bool nullTileReadDefined;
	bool nullTileWriteDefined;
	bool tiledTexture3DAvailable;
};


struct CapsConservativeRasterization {
	unsigned uncertaintyRegion;
	bool postSnapDegenerateTrianglesCulled;
	bool innerInputCoverage;
};

struct CapsResourceHeaps {

};


} // namespace inl::gxapi