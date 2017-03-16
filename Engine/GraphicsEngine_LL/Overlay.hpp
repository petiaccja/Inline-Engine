#pragma once

#include "EntityCollection.hpp"

#include <string>

namespace inl::gxeng {

class OverlayEntity;
class GraphicsEngine;


class Overlay {
public:
	Overlay() = default;
	Overlay(std::string name);
	Overlay(const Overlay&) = delete;
	Overlay& operator=(const Overlay&) = delete;
	virtual ~Overlay();

	void SetName(std::string name);
	const std::string& GetName() const;

	EntityCollection<OverlayEntity>& GetEntities();
	const EntityCollection<OverlayEntity>& GetEntities() const;

private:
	EntityCollection<OverlayEntity> m_entities;

	std::string m_name;
};



} // namespace inl::gxeng
