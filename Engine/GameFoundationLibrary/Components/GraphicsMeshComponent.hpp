#pragma once

#include <GraphicsEngine/Scene/IMeshEntity.hpp>

#include <memory>


namespace inl::gamelib {


struct GraphicsMeshComponent {
	std::unique_ptr<gxeng::IMeshEntity> entity;
	std::shared_ptr<gxeng::IMesh> mesh;
	std::shared_ptr<gxeng::IMaterial> material;
};


} // namespace inl::gamelib