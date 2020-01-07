#pragma once

#include "GraphicsApi_LL/Common.hpp"

namespace inl::gxeng {
class Mesh;
class GraphicsCommandList;
} // namespace inl::gxeng


namespace inl::gxeng::nodes {


gxapi::Rectangle ScissorRect(int screenWidth, int screenHeight);
gxapi::Viewport Viewport(int screenWidth, int screenHeight);
void BindMeshBuffers(GraphicsCommandList& list, const Mesh& mesh);


} // namespace inl::gxeng::nodes