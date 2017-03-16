#include "Overlay.hpp"


namespace inl::gxeng {


Overlay::Overlay(std::string name) : m_name(name) {}


Overlay::~Overlay() {}


void Overlay::SetName(std::string name) {
	m_name = name;
}


const std::string & Overlay::GetName() const {
	return m_name;
}


EntityCollection<OverlayEntity>& Overlay::GetEntities() {
	return m_entities;
}


const EntityCollection<OverlayEntity>& Overlay::GetEntities() const {
	return m_entities;
}


} // namespace inl::gxeng
