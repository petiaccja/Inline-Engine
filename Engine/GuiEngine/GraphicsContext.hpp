#pragma once

#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>


namespace inl::gui {


struct GraphicsContext {
public:
	gxeng::IGraphicsEngine* engine = nullptr;
	gxeng::IScene* scene = nullptr;
};


} // namespace inl::gui