#include "RenderingSystem.hpp"


namespace inl::gamelib {


RenderingSystem::RenderingSystem(gxeng::IGraphicsEngine* engine)
	: m_engine(engine) {}


void RenderingSystem::Update(float elapsed) {
	m_engine->Update(elapsed);
}


} // namespace inl::gamelib
