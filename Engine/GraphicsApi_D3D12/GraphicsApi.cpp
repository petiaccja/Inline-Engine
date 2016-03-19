#include "GraphicsApi.hpp"

#include "CommandQueue.hpp"
#include "CommandAllocator.hpp"
#include "GraphicsCommandList.hpp"
#include "DescriptorHeap.hpp"
#include "NativeCast.hpp"

#include "d3dx12.h"

#include <stdexcept>
#include <cassert>
#include <vector>
#include <list>

namespace inl {
namespace gxapi_dx12 {


gxapi::ICommandQueue* GraphicsApi::CreateCommandQueue(gxapi::CommandQueueDesc desc) {
	ComPtr<ID3D12CommandQueue> native;

	auto nativeDesc = native_cast(desc);
	if (FAILED(m_device->CreateCommandQueue(&nativeDesc, IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command queue.");
	}

	return new CommandQueue{native};
}


gxapi::ICommandAllocator* GraphicsApi::CreateCommandAllocator(gxapi::eCommandListType type) {
	ComPtr<ID3D12CommandAllocator> native;

	if (FAILED(m_device->CreateCommandAllocator(native_cast(type), IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command allocator.");
	}

	return new CommandAllocator{native};
}


gxapi::IGraphicsCommandList* GraphicsApi::CreateGraphicsCommandList(gxapi::CommandListDesc desc) {
	ComPtr<ID3D12GraphicsCommandList> native;

	if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, native_cast(desc.allocator), native_cast(desc.initialState), IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command list.");
	}

	return new GraphicsCommandList{native};
}


gxapi::ICopyCommandList* GraphicsApi::CreateCopyCommandList(gxapi::CommandListDesc desc) {
	//TODO not yet supported
	assert(false);
	return nullptr;
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

	if ((m_device->CreateCommittedResource(&nativeHeapProperties, native_cast(heapFlags), &nativeResourceDesc, native_cast(initialState), pNativeClearValue, IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create commited resource");
	}

	return new Resource{native};
}


gxapi::IRootSignature* GraphicsApi::CreateRootSignature(gxapi::RootSignatureDesc desc) {
	ComPtr<ID3D12RootSignature> native;

	//NATIVE PARAMETERS
	//using list to guarantee that pointers remain valid
	std::list<std::vector<D3D12_DESCRIPTOR_RANGE>> descriptorRangesPerRootParameter;
	std::vector<D3D12_ROOT_PARAMETER> nativeParameters;
	{
		nativeParameters.reserve(desc.numRootParameters);
		for (int i = 0; i < desc.numRootParameters; i++) {
			const auto &source = desc.rootParameters[i];
			D3D12_ROOT_PARAMETER nativeParameter;

			nativeParameter.ShaderVisibility = native_cast(source.shaderVisibility);
			nativeParameter.ParameterType = native_cast(source.type);

			switch (nativeParameter.ParameterType) {
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: {
				const auto& srcTable = source.descriptorTable;
				auto& dstTable = nativeParameter.DescriptorTable;

				descriptorRangesPerRootParameter.push_back(std::vector<D3D12_DESCRIPTOR_RANGE>{});
				auto& nativeRanges = descriptorRangesPerRootParameter.back();
				nativeRanges.reserve(srcTable.numDescriptorRanges);
				for (int i = 0; i< srcTable.numDescriptorRanges; i++) {
					nativeRanges.push_back(native_cast(srcTable.descriptorRanges[i]));
				}

				dstTable.NumDescriptorRanges = nativeRanges.size();
				dstTable.pDescriptorRanges = nativeRanges.data();
			} break;
			case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS: {
				nativeParameter.Constants = native_cast(source.constant);
			} break;
			case D3D12_ROOT_PARAMETER_TYPE_CBV:
			case D3D12_ROOT_PARAMETER_TYPE_SRV:
			case D3D12_ROOT_PARAMETER_TYPE_UAV: {
				nativeParameter.Descriptor = native_cast(source.descriptor);
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
		nativeSamplers.reserve(desc.numStaticSamplers);
		for (int i = 0; i < desc.numStaticSamplers; i++) {
			nativeSamplers.push_back(native_cast(desc.staticSamplers[i]));
		}
	}

	D3D12_ROOT_SIGNATURE_DESC nativeDesc;
	nativeDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; //TODO, default behaviour for now, might be needed to be updated later
	nativeDesc.NumParameters = nativeParameters.size();
	nativeDesc.pParameters = nativeParameters.data();
	nativeDesc.NumStaticSamplers = nativeSamplers.size();
	nativeDesc.pStaticSamplers = nativeSamplers.data();

	ComPtr<ID3DBlob> serializedSignature;
	ComPtr<ID3DBlob> error;
	if (FAILED(D3D12SerializeRootSignature(&nativeDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedSignature, &error))) {
		std::string errorStr;
		errorStr.reserve(error->GetBufferSize());
		for (int i = 0; i < error->GetBufferSize(); i++) {
			errorStr += static_cast<char*>(error->GetBufferPointer())[i];
		}
		throw std::runtime_error("Could not create root signature, error while serializing signature: " + errorStr);
	}

	if (FAILED(m_device->CreateRootSignature(0, serializedSignature->GetBufferPointer(), serializedSignature->GetBufferSize(), IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create root signature, error while executing ID3D12Device::CreateRootSignature.");
	}

	return new RootSignature{native};
}


gxapi::IPipelineState* GraphicsApi::CreateGraphicsPipelineState(gxapi::GraphicsPipelineStateDesc desc) {
	ComPtr<ID3D12PipelineState> native;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC nativeDesc;

	D3D12_STREAM_OUTPUT_DESC nativeStreamOutput;
	static_assert(false, "TODO missing StreamOutputState implementation");

	nativeDesc.pRootSignature = native_cast(desc.rootSignature);
	nativeDesc.VS                     = native_cast(desc.vs);
	nativeDesc.PS                     = native_cast(desc.ps);
	nativeDesc.DS                     = native_cast(desc.ds);
	nativeDesc.HS                     = native_cast(desc.hs);
	nativeDesc.GS                     = native_cast(desc.gs);
	nativeDesc.StreamOutput           = nativeStreamOutput;
	nativeDesc.BlendState			  = native_cast(desc.blending);
	nativeDesc.SampleMask			  = desc.sampleMask;
	nativeDesc.RasterizerState        = desc.rasterizerState;
	nativeDesc.DepthStencilState      = desc.depthStencilState;
	nativeDesc.InputLayout            = desc.inputLayout;
	nativeDesc.IBStripCutValue        = desc.iBStripCutValue;
	nativeDesc.PrimitiveTopologyType  = desc.primitiveTopologyType;
	nativeDesc.NumRenderTargets       = desc.numRenderTargets;
	nativeDesc.RTVFormats             = desc.rTVFormats[8];
	nativeDesc.DSVFormat              = desc.dSVFormat;
	nativeDesc.SampleDesc             = desc.sampleDesc;
	nativeDesc.NodeMask               = desc.nodeMask;
	nativeDesc.CachedPSO              = desc.cachedPSO;
	nativeDesc.Flags                  = desc.flags;



	if (FAILED(m_device->CreateGraphicsPipelineState(&nativeDesc, IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create raphics pipeline state.");
	}

	return new PipelineState{native};
}


gxapi::IDescriptorHeap* GraphicsApi::CreateDescriptorHeap(gxapi::DescriptorHeapDesc desc) {
	ComPtr<ID3D12DescriptorHeap> native;

	auto nativeDesc = native_cast(desc);
	if (FAILED(m_device->CreateDescriptorHeap(&nativeDesc, IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command allocator.");
	}

	return new DescriptorHeap{native};
}


void GraphicsApi::CreateConstantBufferView() {
	static_assert(false, "TODO");
}


void GraphicsApi::CreateDepthStencilView() {
	static_assert(false, "TODO");
}


void GraphicsApi::CreateRenderTargetView() {
	static_assert(false, "TODO");
}


void GraphicsApi::CreateShaderResourceView() {
	static_assert(false, "TODO");
}


} // namespace gxapi_dx12
} // namespace inl
