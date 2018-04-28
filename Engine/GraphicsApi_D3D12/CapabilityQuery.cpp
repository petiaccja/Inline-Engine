#include "CapabilityQuery.hpp"
#include "NativeCast.hpp"


namespace inl::gxapi_dx12 {

using namespace gxapi;


CapabilityQuery::CapabilityQuery(Microsoft::WRL::ComPtr<ID3D12Device> device) {
	m_device = std::move(device);

	m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
}


CapsResourceBinding CapabilityQuery::QueryResourceBinding() const {
	switch (options.ResourceBindingTier) {
		case D3D12_RESOURCE_BINDING_TIER_1: return CapsResourceBinding::Dx12Tier1();
		case D3D12_RESOURCE_BINDING_TIER_2: return CapsResourceBinding::Dx12Tier2();
		case D3D12_RESOURCE_BINDING_TIER_3: return CapsResourceBinding::Dx12Tier3();
		default: assert(false); return {};
	}
}


CapsTiledResources CapabilityQuery::QueryTiledResources() const {
	switch (options.TiledResourcesTier) {
		case D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED: return CapsTiledResources{};
		case D3D12_TILED_RESOURCES_TIER_1: return CapsTiledResources::Dx12Tier1();
		case D3D12_TILED_RESOURCES_TIER_2: return CapsTiledResources::Dx12Tier2();
		case D3D12_TILED_RESOURCES_TIER_3: return CapsTiledResources::Dx12Tier3();
		default: assert(false); return {};
	}
}


CapsConservativeRasterization CapabilityQuery::QueryConservativeRasterization() const {
	switch (options.ConservativeRasterizationTier) {
		case D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED: return CapsConservativeRasterization{};
		case D3D12_CONSERVATIVE_RASTERIZATION_TIER_1: return CapsConservativeRasterization::Dx12Tier1();
		case D3D12_CONSERVATIVE_RASTERIZATION_TIER_2: return CapsConservativeRasterization::Dx12Tier2();
		case D3D12_CONSERVATIVE_RASTERIZATION_TIER_3: return CapsConservativeRasterization::Dx12Tier3();
		default: assert(false); return {};
	}
}


CapsResourceHeaps CapabilityQuery::QueryResourceHeaps() const {
	switch (options.ResourceHeapTier) {
		case D3D12_RESOURCE_HEAP_TIER_1: return CapsResourceHeaps::Dx12Tier1();
		case D3D12_RESOURCE_HEAP_TIER_2: return CapsResourceHeaps::Dx12Tier2();
		default: assert(false); return {};
	}
}


CapsAdditional CapabilityQuery::QueryAdditional() const {
	CapsAdditional caps;
	
	HRESULT hr;
	D3D12_FEATURE_DATA_SHADER_MODEL shader;
	shader.HighestShaderModel = (D3D_SHADER_MODEL)0x61; // D3D_SHADER_MODEL_6_1; // AppVeyor does not take it
	D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT address;

	hr = m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shader, sizeof(shader));
	hr = m_device->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &address, sizeof(address));

	caps.rovsSupported = options.ROVsSupported == TRUE;
	caps.virtualAddressBitsPerResource = address.MaxGPUVirtualAddressBitsPerResource;
	caps.virtualAddressBitsPerProcess = address.MaxGPUVirtualAddressBitsPerProcess;
	caps.shaderModelMajor = (unsigned)shader.HighestShaderModel >> 4;
	caps.shaderModelMinor = (unsigned)shader.HighestShaderModel & 0b1111;

	return caps;
}


CapsLimits CapabilityQuery::QueryLimits() const {
	CapsLimits limits;
	limits.texture1DSize = { 16384 };
	limits.texture2DSize = { 16384, 16384 };
	limits.texture3DSize = { 16384, 16384, 2048 };
	limits.textureRepeat = 16384;
	limits.anisotropy = 16;
	limits.primitiveCount = (1ull << 32) - 1;
	limits.vertexCount = (1ull << 32) - 1;
	limits.inputSlots = 32;
	limits.multipleRenderTargets = 8;

	return limits;
}


eCapsFormatUsage CapabilityQuery::QueryFormat(eFormat format) const {
	DXGI_FORMAT nativeFormat = native_cast(format);
	D3D12_FEATURE_DATA_FORMAT_SUPPORT data;
	data.Format = nativeFormat;
	data.Support1 = D3D12_FORMAT_SUPPORT1_NONE;
	data.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
	m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &data, sizeof(data));

	eCapsFormatUsage usage;


	if (data.Support1 & D3D12_FORMAT_SUPPORT1_BUFFER)
		usage += eCapsFormatUsage::BUFFER;
	if (data.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER)
		usage += eCapsFormatUsage::VERTEX_BUFFER;
	if (data.Support1 & D3D12_FORMAT_SUPPORT1_SO_BUFFER)
		usage += eCapsFormatUsage::SO_BUFFER;
	if (data.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE1D)
		usage += eCapsFormatUsage::TEXTURE_1D;
	if (data.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D)
		usage += eCapsFormatUsage::TEXTURE_2D;
	if (data.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE3D)
		usage += eCapsFormatUsage::TEXTURE_3D;
	if (data.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURECUBE)
		usage += eCapsFormatUsage::TEXTURE_CUBE;


	if (data.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD)
		usage += eCapsFormatUsage::SAMPLE;
	if (data.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)
		usage += eCapsFormatUsage::SAMPLE_LINEAR;
	if (data.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)
		usage += eCapsFormatUsage::RENDER_TARGET;
	if (data.Support1 & D3D12_FORMAT_SUPPORT1_BLENDABLE)
		usage += eCapsFormatUsage::RENDER_TARGET_BLEND;
	if (data.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
		usage += eCapsFormatUsage::DEPTH_STENCIL;



	if (data.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD)
		usage += eCapsFormatUsage::UNORDERED_ACCESS_LOAD;
	if (data.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE)
		usage += eCapsFormatUsage::UNORDERED_ACCESS_STORE;
	D3D12_FORMAT_SUPPORT2 atomic =
		D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_ADD
		| D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_BITWISE_OPS
		| D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_COMPARE_STORE_OR_COMPARE_EXCHANGE
		| D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_EXCHANGE
		| D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_SIGNED_MIN_OR_MAX
		| D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_UNSIGNED_MIN_OR_MAX;
	if ((data.Support1 & atomic) == atomic)
		usage += eCapsFormatUsage::UNORDERED_ACCESS_ATOMIC;

	return usage;
}



bool CapabilityQuery::SupportsAll(const CapsRequirementSet& requiredFeatures) const {
	CapsResourceBinding resourceBinding = QueryResourceBinding();
	CapsTiledResources tiledResources = QueryTiledResources();
	CapsConservativeRasterization conservativeRasterization = QueryConservativeRasterization();
	CapsResourceHeaps resourceHeaps = QueryResourceHeaps();
	CapsAdditional additional = QueryAdditional();
	CapsLimits limits = QueryLimits();

	bool featuresSupported =
		resourceBinding >= requiredFeatures.resourceBinding
		&& tiledResources >= requiredFeatures.tiledResources
		&& conservativeRasterization >= requiredFeatures.conservativeRasterization
		&& resourceHeaps >= requiredFeatures.resourceHeaps
		&& additional >= requiredFeatures.additional
		&& limits >= requiredFeatures.limits;

	bool formatsSupported = true;
	for (const auto& fmtQuery : requiredFeatures.formats) {
		eCapsFormatUsage fmtCaps = QueryFormat(fmtQuery.first);
		formatsSupported = formatsSupported && fmtCaps.Contains(fmtQuery.second);
	}

	return featuresSupported && formatsSupported;
}




} // namespace inl::gxapi_dx12