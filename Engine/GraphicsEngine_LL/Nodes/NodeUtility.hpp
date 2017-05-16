#pragma once

#include <GraphicsApi_LL/Common.hpp>


namespace inl::gxeng::nodes {

/// <summary>
/// Converts any format to its depth stencil equivalent.
/// <para/>
/// The input format can be typed color, depth stencil, or typeless.
/// If there is no corresponding depth stencil format, UNKNOWN is returned.
/// </summary>
gxapi::eFormat FormatAnyToDepthStencil(gxapi::eFormat sourceFormat);

/// <summary>
/// Converts any depth format to its color equivalent.
/// <para/>
/// The input format can be depth stencil, color, or typeless.
/// If there is no corresponding depth format, UNKNOWN is returned.
/// If the input is already a color format that has a corresponding depth format than it is returned unchanged.
/// </summary>
gxapi::eFormat FormatDepthToColor(gxapi::eFormat sourceFormat);

} // namespace inl::gxeng::nodes


