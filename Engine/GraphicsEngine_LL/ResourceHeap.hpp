#pragma once

#include "HighLevelDescHeap.hpp"
#include "GpuBuffer.hpp"
#include "SubresourceID.hpp"

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"
#include "../GraphicsApi_LL/ISwapChain.hpp"
#include "../BaseLibrary/ScalarLiterals.hpp"
#include "../BaseLibrary/RingBuffer.hpp"

#include <unordered_map>

namespace inl {
namespace gxeng {

class CopyCommandList;
class GenericResource;
class LinearBuffer;
class Texture2D;


enum class eResourceHeapType { CRITICAL };


namespace impl {

using namespace exc::prefix;


struct InitialResourceParameters {
	InitialResourceParameters(DescriptorReference&& d) : desc(std::move(d)) {}

	DescriptorReference desc;
	gxapi::IResource* resource;		
	bool residency;
};


class CriticalBufferHeap {
public:
	CriticalBufferHeap(gxapi::IGraphicsApi* graphicsApi);
	InitialResourceParameters Allocate(DescriptorReference&& viewRef, gxapi::ResourceDesc desc);

protected:
	gxapi::IGraphicsApi* m_graphicsApi;
};


#if 0 //TODO
class StreamedBufferHeap {
public:
	gxapi::IResource* Allocate(GenericResource* owner, gxapi::ResourceDesc desc);
	void ReleaseUnderlying(GenericResource* owner) override;
};
#endif


//TODO
#if 0

class OverlappedBufferHeap {
public:
	//gxapi::IResource* Allocate(GenericResource* owner, gxapi::ResourceDesc desc, gxapi::IResource* shared = nullptr);
	void ReleaseUnderlying(GenericResource* owner) override;
};
#endif


} // namespace impl


class BackBufferHeap
{
public:
	BackBufferHeap(gxapi::IGraphicsApi* graphicsApi, gxapi::ISwapChain* swapChain);

	Texture2D& GetBackBuffer(unsigned index);
	
protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	gxapi::ISwapChain* m_swapChain;

	std::unique_ptr<gxapi::IDescriptorHeap> m_descriptorHeap;
	std::vector<Texture2D> m_backBuffers;
};


class UploadHeap {
public:
	struct UploadDescription {
		UploadDescription(GenericResource&& source, GenericResource* pDestination) :
			source(std::move(source)), pDestination(pDestination) {}

		GenericResource source;
		GenericResource* pDestination;
	};
public:
	UploadHeap(gxapi::IGraphicsApi* graphicsApi);

	//void UploadToResource(gxeng::CopyCommandList& cmdList, LinearBuffer& target, const void* data, size_t size);
	void UploadToResource(LinearBuffer& target, const void* data, size_t size);

	std::vector<UploadDescription>& _GetQueuedUploads();

protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	std::vector<UploadDescription> m_uploadQueue;
};


} // namespace gxeng
} // namespace inl
