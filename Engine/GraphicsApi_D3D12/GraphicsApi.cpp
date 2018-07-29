#include "GraphicsApi.hpp"

#include "CommandQueue.hpp"
#include "CommandAllocator.hpp"
#include "CommandList.hpp"
#include "DescriptorHeap.hpp"
#include "NativeCast.hpp"
#include "ExceptionExpansions.hpp"
#include "CapabilityQuery.hpp"

#include "../GraphicsApi_LL/Exception.hpp"

#include "d3dx12.h"

#include <stdexcept>
#include <cassert>
#include <vector>
#include <array>
#include <list>

namespace inl {
namespace gxapi_dx12 {


GraphicsApi::GraphicsApi(Microsoft::WRL::ComPtr<ID3D12Device> device) : m_device(device) {
	m_device->QueryInterface(IID_PPV_ARGS(&m_debugDevice));
}

GraphicsApi::~GraphicsApi() {
	//if (m_debugDevice) {
	//	m_debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	//}
}

void GraphicsApi::ReportLiveObjects() const {
	OutputDebugStringW(L"Live Direct3D 12 objects:\n");
	if (m_debugDevice) {
		m_debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	}
}


gxapi::ICommandQueue* GraphicsApi::CreateCommandQueue(gxapi::CommandQueueDesc desc) {
	ComPtr<ID3D12CommandQueue> native;

	auto nativeDesc = native_cast(desc);
	ThrowIfFailed(m_device->CreateCommandQueue(&nativeDesc, IID_PPV_ARGS(&native)));

	return new CommandQueue{ native };
}


gxapi::ICommandAllocator* GraphicsApi::CreateCommandAllocator(gxapi::eCommandListType type) {
	ComPtr<ID3D12CommandAllocator> native;

	ThrowIfFailed(m_device->CreateCommandAllocator(native_cast(type), IID_PPV_ARGS(&native)));

	return new CommandAllocator{ native, type };
}


gxapi::IGraphicsCommandList* GraphicsApi::CreateGraphicsCommandList(gxapi::CommandListDesc desc) {
	ComPtr<ID3D12GraphicsCommandList> native;

	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, native_cast(desc.allocator), native_cast(desc.initialState), IID_PPV_ARGS(&native)));

	return new GraphicsCommandList{ native };
}


gxapi::IComputeCommandList* GraphicsApi::CreateComputeCommandList(gxapi::CommandListDesc desc) {
	ComPtr<ID3D12GraphicsCommandList> native;

	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, native_cast(desc.allocator), native_cast(desc.initialState), IID_PPV_ARGS(&native)));

	return new ComputeCommandList{ native };
}


gxapi::ICopyCommandList* GraphicsApi::CreateCopyCommandList(gxapi::CommandListDesc desc) {
	ComPtr<ID3D12GraphicsCommandList> native;

	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, native_cast(desc.allocator), native_cast(desc.initialState), IID_PPV_ARGS(&native)));

	return new CopyCommandList{ native };
}


gxapi::ICommandList* GraphicsApi::CreateCommandList(gxapi::eCommandListType type, gxapi::CommandListDesc desc) {
	ComPtr<ID3D12GraphicsCommandList> native;

	ThrowIfFailed(m_device->CreateCommandList(0, native_cast(type), native_cast(desc.allocator), native_cast(desc.initialState), IID_PPV_ARGS(&native)));
	switch (type)
	{
	case inl::gxapi::eCommandListType::COPY:
		return new CopyCommandList(native);
	case inl::gxapi::eCommandListType::COMPUTE:
		return new ComputeCommandList(native);
	case inl::gxapi::eCommandListType::GRAPHICS:
		return new GraphicsCommandList(native);
	case inl::gxapi::eCommandListType::BUNDLE:
		throw InvalidArgumentException("Bundles are not supported.");
	default:
		assert(false);
		throw InvalidArgumentException("What memory garbage did you even specify dumbass?", std::to_string((long long)type));
	}
}


gxapi::IResource* GraphicsApi::CreateCommittedResource(gxapi::HeapProperties heapProperties,
													   gxapi::eHeapFlags heapFlags,
													   gxapi::ResourceDesc desc,
													   gxapi::eResourceState initialState,
													   gxapi::ClearValue* clearValue) {

	ComPtr<ID3D12Resource> native;

	D3D12_HEAP_PROPERTIES nativeHeapProperties = native_cast(heapProperties);
	D3D12_RESOURCE_DESC nativeResourceDesc = native_cast(desc);

	D3D12_CLEAR_VALUE* pNativeClearValue = nullptr;
	D3D12_CLEAR_VALUE nativeClearValue;
	if (clearValue != nullptr) {
		nativeClearValue = native_cast(*clearValue);
		pNativeClearValue = &nativeClearValue;
	}

	ThrowIfFailed(m_device->CreateCommittedResource(&nativeHeapProperties, native_cast(heapFlags), &nativeResourceDesc, native_cast(initialState), pNativeClearValue, IID_PPV_ARGS(&native)));

	return new Resource{ native, m_device };
}


gxapi::IRootSignature* GraphicsApi::CreateRootSignature(gxapi::RootSignatureDesc desc) {
	ComPtr<ID3D12RootSignature> native;

	//NATIVE PARAMETERS
	//using list to guarantee that pointers remain valid
	std::list<std::vector<D3D12_DESCRIPTOR_RANGE>> descriptorRangesPerRootParameter;
	std::vector<D3D12_ROOT_PARAMETER> nativeParameters;
	{
		nativeParameters.reserve(desc.rootParameters.size());
		for (const auto& source : desc.rootParameters) {
			D3D12_ROOT_PARAMETER nativeParameter;

			nativeParameter.ShaderVisibility = native_cast(source.shaderVisibility);
			nativeParameter.ParameterType = native_cast(source.type);

			switch (nativeParameter.ParameterType) {
				case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
				{
					const auto& srcTable = source.As<gxapi::RootParameterDesc::DESCRIPTOR_TABLE>();
					auto& dstTable = nativeParameter.DescriptorTable;

					descriptorRangesPerRootParameter.push_back(std::vector<D3D12_DESCRIPTOR_RANGE>{});
					auto& nativeRanges = descriptorRangesPerRootParameter.back();
					nativeRanges.reserve(srcTable.ranges.size());
					for (unsigned i = 0; i < srcTable.ranges.size(); i++) {
						nativeRanges.push_back(native_cast(srcTable.ranges[i]));
					}

					dstTable.NumDescriptorRanges = (UINT)nativeRanges.size();
					dstTable.pDescriptorRanges = nativeRanges.data();
				} break;
				case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
				{
					nativeParameter.Constants = native_cast(source.As<gxapi::RootParameterDesc::CONSTANT>());
				} break;
				case D3D12_ROOT_PARAMETER_TYPE_CBV:
				case D3D12_ROOT_PARAMETER_TYPE_SRV:
				case D3D12_ROOT_PARAMETER_TYPE_UAV:
				{
					nativeParameter.Descriptor = native_cast(source.As<gxapi::RootParameterDesc::CBV>());
				} break;
				default:
					assert(false);
					break;
			}

			nativeParameters.push_back(nativeParameter);
		}
	}

	//NATIVE STATIC SAMPLERS
	std::vector<D3D12_STATIC_SAMPLER_DESC> nativeSamplers;
	{
		nativeSamplers.reserve(desc.staticSamplers.size());
		for (const auto& staticSampler : desc.staticSamplers) {
			nativeSamplers.push_back(native_cast(staticSampler));
		}
	}

	D3D12_ROOT_SIGNATURE_DESC nativeDesc;
	//TODO might be needed to be updated later
	nativeDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	nativeDesc.NumParameters = (UINT)nativeParameters.size();
	nativeDesc.pParameters = nativeParameters.size() > 0 ? nativeParameters.data() : nullptr;
	nativeDesc.NumStaticSamplers = (UINT)nativeSamplers.size();
	nativeDesc.pStaticSamplers = nativeSamplers.size() > 0 ? nativeSamplers.data() : nullptr;

	ComPtr<ID3DBlob> serializedSignature;
	ComPtr<ID3DBlob> error;
	if (FAILED(D3D12SerializeRootSignature(&nativeDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedSignature, &error))) {
		std::string errorStr;
		errorStr.reserve(error->GetBufferSize());
		for (unsigned i = 0; i < error->GetBufferSize(); i++) {
			errorStr += static_cast<char*>(error->GetBufferPointer())[i];
		}
		throw InvalidArgumentException("Could not create root signature, error while serializing signature: " + errorStr);
	}

	ThrowIfFailed(m_device->CreateRootSignature(0, serializedSignature->GetBufferPointer(), serializedSignature->GetBufferSize(), IID_PPV_ARGS(&native)));

	return new RootSignature{ native };
}


gxapi::IPipelineState* GraphicsApi::CreateGraphicsPipelineState(const gxapi::GraphicsPipelineStateDesc& desc) {
	ComPtr<ID3D12PipelineState> native;

	gxapi::GraphicsPipelineStateDesc desc2{ desc };

	D3D12_GRAPHICS_PIPELINE_STATE_DESC nativeDesc = {};

	D3D12_STREAM_OUTPUT_DESC nativeStreamOutput;
	static_assert(sizeof(decltype(desc.streamOutput)) <= 1, "If Stream output is implemented, it should be handled propery here.");
	nativeStreamOutput.NumEntries = 0;
	nativeStreamOutput.NumStrides = 0;
	nativeStreamOutput.pBufferStrides = nullptr;
	nativeStreamOutput.pSODeclaration = nullptr;
	nativeStreamOutput.RasterizedStream = 0;

	std::vector<D3D12_INPUT_ELEMENT_DESC> nativeInputElements;
	nativeInputElements.reserve(desc.inputLayout.numElements);

	for (unsigned i = 0; i < desc.inputLayout.numElements; i++) {
		nativeInputElements.push_back(native_cast(desc.inputLayout.elements[i]));
	}

	D3D12_INPUT_LAYOUT_DESC nativeInputLayout;
	nativeInputLayout.NumElements = (UINT)nativeInputElements.size();
	nativeInputLayout.pInputElementDescs = nativeInputElements.data();


	nativeDesc.pRootSignature = native_cast(desc.rootSignature);
	nativeDesc.VS = native_cast(desc.vs);
	nativeDesc.PS = native_cast(desc.ps);
	nativeDesc.DS = native_cast(desc.ds);
	nativeDesc.HS = native_cast(desc.hs);
	nativeDesc.GS = native_cast(desc.gs);
	nativeDesc.StreamOutput = nativeStreamOutput;
	nativeDesc.BlendState = native_cast(desc.blending);
	nativeDesc.SampleMask = desc.blendSampleMask;
	nativeDesc.RasterizerState = native_cast(desc.rasterization);
	nativeDesc.DepthStencilState = native_cast(desc.depthStencilState);
	nativeDesc.InputLayout = nativeInputLayout;
	nativeDesc.IBStripCutValue = native_cast(desc.triangleStripCutIndex);
	nativeDesc.PrimitiveTopologyType = native_cast(desc.primitiveTopologyType);
	nativeDesc.NumRenderTargets = desc.numRenderTargets;

	for (auto& v : nativeDesc.RTVFormats) {
		v = DXGI_FORMAT_UNKNOWN;
	}
	for (unsigned i = 0; i < desc.numRenderTargets; i++) {
		nativeDesc.RTVFormats[i] = native_cast(desc.renderTargetFormats[i]);
	}

	nativeDesc.DSVFormat = native_cast(desc.depthStencilFormat);
	nativeDesc.SampleDesc.Count = desc.multisampleCount;
	nativeDesc.SampleDesc.Quality = desc.multisampleQuality;
	nativeDesc.NodeMask = 0;
	nativeDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	nativeDesc.CachedPSO.pCachedBlob = nullptr;
	nativeDesc.Flags = desc.addDebugInfo ? D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG : D3D12_PIPELINE_STATE_FLAG_NONE;


	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&nativeDesc, IID_PPV_ARGS(&native)), "While creating graphics PSO");

	return new PipelineState{ native };
}


gxapi::IPipelineState* GraphicsApi::CreateComputePipelineState(const gxapi::ComputePipelineStateDesc& desc) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC nativeDesc;
	nativeDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	nativeDesc.CachedPSO.pCachedBlob = nullptr;
	nativeDesc.CS.pShaderBytecode = desc.cs.shaderByteCode;
	nativeDesc.CS.BytecodeLength = desc.cs.sizeOfByteCode;
	nativeDesc.Flags = desc.addDebugInfo ? D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG : D3D12_PIPELINE_STATE_FLAG_NONE;
	nativeDesc.NodeMask = 0;
	nativeDesc.pRootSignature = native_cast(desc.rootSignature);

	ComPtr<ID3D12PipelineState> native;
	ThrowIfFailed(m_device->CreateComputePipelineState(&nativeDesc, IID_PPV_ARGS(&native)), "While creating compute PSO");

	return new PipelineState{ native };
}


gxapi::IDescriptorHeap* GraphicsApi::CreateDescriptorHeap(gxapi::DescriptorHeapDesc desc) {
	ComPtr<ID3D12DescriptorHeap> native;

	auto nativeDesc = native_cast(desc);
	ThrowIfFailed(m_device->CreateDescriptorHeap(&nativeDesc, IID_PPV_ARGS(&native)));

	return new DescriptorHeap{ native, m_device.Get() };
}


void GraphicsApi::CreateConstantBufferView(gxapi::ConstantBufferViewDesc desc,
										   gxapi::DescriptorHandle destination)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC nativeDesc = native_cast(desc);
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateConstantBufferView(&nativeDesc, nativeCPUHandle);
}


void GraphicsApi::CreateDepthStencilView(gxapi::DepthStencilViewDesc desc,
										 gxapi::DescriptorHandle destination)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC nativeDesc = native_cast(desc);
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateDepthStencilView(nullptr, &nativeDesc, nativeCPUHandle);
}


void GraphicsApi::CreateDepthStencilView(const gxapi::IResource* resource,
										 gxapi::DescriptorHandle destination)
{
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateDepthStencilView(const_cast<ID3D12Resource*>(native_cast(resource)), nullptr, nativeCPUHandle);
}


void GraphicsApi::CreateDepthStencilView(const gxapi::IResource* resource,
										 gxapi::DepthStencilViewDesc desc,
										 gxapi::DescriptorHandle destination)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC nativeDesc = native_cast(desc);
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateDepthStencilView(const_cast<ID3D12Resource*>(native_cast(resource)), &nativeDesc, nativeCPUHandle);
}


void GraphicsApi::CreateRenderTargetView(const gxapi::IResource* resource,
										 gxapi::RenderTargetViewDesc desc,
										 gxapi::DescriptorHandle destination)
{
	D3D12_RENDER_TARGET_VIEW_DESC nativeDesc = native_cast(desc);
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateRenderTargetView(const_cast<ID3D12Resource*>(native_cast(resource)), &nativeDesc, nativeCPUHandle);
}


void GraphicsApi::CreateRenderTargetView(const gxapi::IResource* resource,
										 gxapi::DescriptorHandle destination)
{
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateRenderTargetView(const_cast<ID3D12Resource*>(native_cast(resource)), nullptr, nativeCPUHandle);
}


void GraphicsApi::CreateShaderResourceView(gxapi::ShaderResourceViewDesc desc,
										   gxapi::DescriptorHandle destination)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC nativeDesc = native_cast(desc);
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateShaderResourceView(nullptr, &nativeDesc, nativeCPUHandle);
}


void GraphicsApi::CreateShaderResourceView(const gxapi::IResource* resource,
										   gxapi::DescriptorHandle destination)
{
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateShaderResourceView(const_cast<ID3D12Resource*>(native_cast(resource)), nullptr, nativeCPUHandle);
}


void GraphicsApi::CreateShaderResourceView(const gxapi::IResource* resource,
										   gxapi::ShaderResourceViewDesc desc,
										   gxapi::DescriptorHandle destination)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC nativeDesc = native_cast(desc);
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateShaderResourceView(const_cast<ID3D12Resource*>(native_cast(resource)), &nativeDesc, nativeCPUHandle);
}


void GraphicsApi::CreateUnorderedAccessView(gxapi::UnorderedAccessViewDesc descriptor,
											gxapi::DescriptorHandle destination)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC nativeDesc = native_cast(descriptor);
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateUnorderedAccessView(nullptr, nullptr, &nativeDesc, nativeCPUHandle);
}


void GraphicsApi::CreateUnorderedAccessView(const gxapi::IResource* resource,
											gxapi::DescriptorHandle destination)
{
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateUnorderedAccessView(const_cast<ID3D12Resource*>(native_cast(resource)), nullptr, nullptr, nativeCPUHandle);
}


void GraphicsApi::CreateUnorderedAccessView(const gxapi::IResource* resource,
											gxapi::UnorderedAccessViewDesc descriptor,
											gxapi::DescriptorHandle destination)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC nativeDesc = native_cast(descriptor);
	D3D12_CPU_DESCRIPTOR_HANDLE nativeCPUHandle;
	nativeCPUHandle.ptr = native_cast_ptr(destination.cpuAddress);
	m_device->CreateUnorderedAccessView(const_cast<ID3D12Resource*>(native_cast(resource)), nullptr, &nativeDesc, nativeCPUHandle);
}


void GraphicsApi::CopyDescriptors(size_t numSrcDescRanges,
								  gxapi::DescriptorHandle * srcRangeStarts,
								  size_t numDstDescRanges,
								  gxapi::DescriptorHandle * dstRangeStarts,
								  uint32_t * rangeCounts,
								  gxapi::eDescriptorHeapType descHeapsType)
{
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> nativeDstRangeStarts(numDstDescRanges);
	for (int i = 0; i < numDstDescRanges; i++) {
		nativeDstRangeStarts[i].ptr = native_cast_ptr(dstRangeStarts[i].cpuAddress);
	}
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> nativeSrcRangeStarts(numSrcDescRanges);
	for (int i = 0; i < numSrcDescRanges; i++) {
		nativeSrcRangeStarts[i].ptr = native_cast_ptr(srcRangeStarts[i].cpuAddress);
	}

	m_device->CopyDescriptors(
		(unsigned)nativeDstRangeStarts.size(),
		nativeDstRangeStarts.data(),
		rangeCounts,
		(unsigned)nativeSrcRangeStarts.size(),
		nativeSrcRangeStarts.data(),
		nullptr,
		native_cast(descHeapsType)
	);
}


void GraphicsApi::CopyDescriptors(size_t numSrcDescRanges,
								  gxapi::DescriptorHandle * srcRangeStarts,
								  uint32_t * srcRangeLengths,
								  size_t numDstDescRanges,
								  gxapi::DescriptorHandle * dstRangeStarts,
								  uint32_t * dstRangeLengths,
								  gxapi::eDescriptorHeapType descHeapsType)
{
	// Thread local to avoid allocations on each call.
	thread_local std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> nativeDstRangeStarts;
	thread_local std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> nativeSrcRangeStarts;
	
	nativeDstRangeStarts.resize(numDstDescRanges);
	nativeSrcRangeStarts.resize(numSrcDescRanges);

	for (int i = 0; i < numDstDescRanges; i++) {
		nativeDstRangeStarts[i].ptr = native_cast_ptr(dstRangeStarts[i].cpuAddress);
	}
	for (int i = 0; i < numSrcDescRanges; i++) {
		nativeSrcRangeStarts[i].ptr = native_cast_ptr(srcRangeStarts[i].cpuAddress);
	}

	m_device->CopyDescriptors(
		(unsigned)nativeDstRangeStarts.size(),
		nativeDstRangeStarts.data(),
		dstRangeLengths,
		(unsigned)nativeSrcRangeStarts.size(),
		nativeSrcRangeStarts.data(),
		srcRangeLengths,
		native_cast(descHeapsType)
	);
}


void GraphicsApi::CopyDescriptors(gxapi::DescriptorHandle srcStart,
								  gxapi::DescriptorHandle dstStart,
								  size_t rangeCount,
								  gxapi::eDescriptorHeapType descHeapsType)
{
	D3D12_CPU_DESCRIPTOR_HANDLE nativeDstRangeStart;
	nativeDstRangeStart.ptr = native_cast_ptr(dstStart.cpuAddress);

	D3D12_CPU_DESCRIPTOR_HANDLE nativeSrcRangeStart;
	nativeSrcRangeStart.ptr = native_cast_ptr(srcStart.cpuAddress);

	m_device->CopyDescriptorsSimple((unsigned)rangeCount, nativeDstRangeStart, nativeSrcRangeStart, native_cast(descHeapsType));
}


gxapi::IFence * GraphicsApi::CreateFence(uint64_t initialValue)
{
	ComPtr<ID3D12Fence> native;
	D3D12_FENCE_FLAGS flags = D3D12_FENCE_FLAG_NONE;
	ThrowIfFailed(m_device->CreateFence(initialValue, flags, IID_PPV_ARGS(&native)));
	return new Fence(native);
}


void GraphicsApi::MakeResident(const std::vector<gxapi::IResource*>& objects) {
	if (objects.size() == 0) {
		return;
	}

	std::vector<ID3D12Pageable*> nativeObjects;
	nativeObjects.reserve(objects.size());

	for (auto curr : objects) {
		nativeObjects.push_back(native_cast(curr));
	}

	ThrowIfFailed(m_device->MakeResident((unsigned)nativeObjects.size(), nativeObjects.data()));
}


void GraphicsApi::Evict(const std::vector<gxapi::IResource*>& objects) {
	if (objects.size() == 0) {
		return;
	}

	std::vector<ID3D12Pageable*> nativeObjects;
	nativeObjects.reserve(objects.size());

	for (auto curr : objects) {
		nativeObjects.push_back(native_cast(curr));
	}

	ThrowIfFailed(m_device->Evict((unsigned)nativeObjects.size(), nativeObjects.data()));
}


gxapi::ICapabilityQuery* GraphicsApi::GetCapabilityQuery() const {
	return new CapabilityQuery(m_device);
}



} // namespace gxapi_dx12
} // namespace inl
