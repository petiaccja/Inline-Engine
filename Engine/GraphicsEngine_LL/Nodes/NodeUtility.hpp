#pragma once

#include <GraphicsApi_LL/Common.hpp>


namespace inl::gxeng::nodes {

/// <summary>
/// Converts any format to its depth stencil equivalent.
/// <para/>
/// The input format can be typed color, depth stencil, or typeless.
/// If there is no corresponding depth stencil format, UNKNOWN is returned.
/// </summary>
static gxapi::eFormat FormatAnyToDepthStencil(gxapi::eFormat sourceFormat) {
	using gxapi::eFormat;

	switch (sourceFormat) {
	case eFormat::R32G8X24_TYPELESS:
		return eFormat::D32_FLOAT_S8X24_UINT;
	case eFormat::D32_FLOAT_S8X24_UINT:
		return eFormat::D32_FLOAT_S8X24_UINT;
	case eFormat::R32_FLOAT_X8X24_TYPELESS:
		return eFormat::D32_FLOAT_S8X24_UINT;
	case eFormat::X32_TYPELESS_G8X24_UINT:
		return eFormat::D32_FLOAT_S8X24_UINT;

	case eFormat::R32_TYPELESS:
		return eFormat::D32_FLOAT;
	case eFormat::D32_FLOAT:
		return eFormat::D32_FLOAT;
	case eFormat::R32_FLOAT:
		return eFormat::D32_FLOAT;
	case eFormat::R32_UINT:
		return eFormat::D32_FLOAT;
	case eFormat::R32_SINT:
		return eFormat::D32_FLOAT;

	case eFormat::R24G8_TYPELESS:
		return eFormat::D24_UNORM_S8_UINT;
	case eFormat::D24_UNORM_S8_UINT:
		return eFormat::D24_UNORM_S8_UINT;
	case eFormat::R24_UNORM_X8_TYPELESS:
		return eFormat::D24_UNORM_S8_UINT;
	case eFormat::X24_TYPELESS_G8_UINT:
		return eFormat::D24_UNORM_S8_UINT;

	case eFormat::R16_TYPELESS:
		return eFormat::D16_UNORM;
	case eFormat::R16_FLOAT:
		return eFormat::D16_UNORM;
	case eFormat::D16_UNORM:
		return eFormat::D16_UNORM;
	case eFormat::R16_UNORM:
		return eFormat::D16_UNORM;
	case eFormat::R16_UINT:
		return eFormat::D16_UNORM;
	case eFormat::R16_SNORM:
		return eFormat::D16_UNORM;
	case eFormat::R16_SINT:
		return eFormat::D16_UNORM;
	default:
		break;
	}

	return eFormat::UNKNOWN;
}

/// <summary>
/// Converts any depth format to its color equivalent.
/// <para/>
/// The input format can be depth stencil, color, or typeless.
/// If there is no corresponding depth format, UNKNOWN is returned.
/// If the input is already a color format that has a corresponding depth format than it is returned unchanged.
/// </summary>
static gxapi::eFormat FormatDepthToColor(gxapi::eFormat sourceFormat) {
	using gxapi::eFormat;

	switch (sourceFormat) {
	case eFormat::R32G8X24_TYPELESS:
		return eFormat::R32_FLOAT_X8X24_TYPELESS;
	case eFormat::D32_FLOAT_S8X24_UINT:
		return eFormat::R32_FLOAT_X8X24_TYPELESS;
	case eFormat::R32_FLOAT_X8X24_TYPELESS:
		return eFormat::R32_FLOAT_X8X24_TYPELESS;
	case eFormat::X32_TYPELESS_G8X24_UINT:
		return eFormat::R32_FLOAT_X8X24_TYPELESS;

	case eFormat::R32_TYPELESS:
		return eFormat::R32_FLOAT;
	case eFormat::D32_FLOAT:
		return eFormat::R32_FLOAT;
	case eFormat::R32_FLOAT:
		return eFormat::R32_FLOAT;
	case eFormat::R32_UINT:
		return eFormat::R32_UINT;
	case eFormat::R32_SINT:
		return eFormat::R32_SINT;

	case eFormat::R24G8_TYPELESS:
		return eFormat::R24_UNORM_X8_TYPELESS;
	case eFormat::D24_UNORM_S8_UINT:
		return eFormat::R24_UNORM_X8_TYPELESS;
	case eFormat::R24_UNORM_X8_TYPELESS:
		return eFormat::R24_UNORM_X8_TYPELESS;
	case eFormat::X24_TYPELESS_G8_UINT:
		return eFormat::R24_UNORM_X8_TYPELESS;

	case eFormat::R16_TYPELESS:
		return eFormat::R16_UNORM;
	case eFormat::R16_FLOAT:
		return eFormat::R16_FLOAT;
	case eFormat::D16_UNORM:
		return eFormat::R16_UNORM;
	case eFormat::R16_UNORM:
		return eFormat::R16_UNORM;
	case eFormat::R16_UINT:
		return eFormat::R16_UINT;
	case eFormat::R16_SNORM:
		return eFormat::R16_SNORM;
	case eFormat::R16_SINT:
		return eFormat::R16_SINT;
	default:
		break;
	}

	return eFormat::UNKNOWN;
}

} // namespace inl::gxeng::nodes


