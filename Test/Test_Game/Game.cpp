#include "Game.hpp"


Game::Game(inl::Window& window)
	: m_modules(window.GetNativeHandle()),
	  m_assetCaches(m_modules.GetGraphicsEngine()) {

}
