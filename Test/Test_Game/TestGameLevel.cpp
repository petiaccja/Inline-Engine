#include "TestGameLevel.hpp"

#include <BaseLibrary/Exception/Exception.hpp>


void TestGameLevel::SetWorld(inl::game::World& world) {
	m_world = &world;
}


void TestGameLevel::Load(std::string_view levelName) {
	throw inl::NotImplementedException();
}


void TestGameLevel::Save(std::ostream& file) {
	throw inl::NotImplementedException();
}


void TestGameLevel::Reset() {
	m_world = nullptr;
}
