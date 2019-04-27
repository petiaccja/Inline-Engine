#pragma once

#include <GraphicsEngine/Scene/IMeshEntity.hpp>

#include <memory>


namespace inl::gamelib {


struct GraphicsComponent {
	std::unique_ptr<gxeng::IMeshEntity> entity;
};


} // namespace inl::gamelib