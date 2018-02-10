#pragma once

#include "../GraphicsApi_LL/HardwareCapability.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"



namespace inl::gxapi_dx12 {


class CapabilityQuery : public gxapi::ICapabilityQuery {
public:
	CapabilityQuery(Microsoft::WRL::ComPtr<ID3D12Device> device);

	gxapi::CapsResourceBinding QueryResourceBinding() const override;
	gxapi::CapsTiledResources QueryTiledResources() const override;
	gxapi::CapsConservativeRasterization QueryConservativeRasterization() const override;
	gxapi::CapsResourceHeaps QueryResourceHeaps() const override;
	gxapi::CapsAdditional QueryAdditional() const override;
	gxapi::CapsLimits QueryLimits() const override;
	gxapi::eCapsFormatUsage QueryFormat(gxapi::eFormat format) const override;

	bool SupportsAll(const gxapi::CapsRequirementSet& requiredFeatures) const override;

private:
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;

	D3D12_FEATURE_DATA_D3D12_OPTIONS options;
};


} // namespace inl::gxapi_dx12