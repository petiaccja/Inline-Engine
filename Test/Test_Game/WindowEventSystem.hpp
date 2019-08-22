#pragma once

#include <GameLogic/System.hpp>
#include <BaseLibrary/Platform/Window.hpp>


class WindowEventSystem : public inl::game::SpecificSystem<WindowEventSystem> {
public:
	void Update(float elapsed) override;
	void SetWindows(std::vector<inl::Window*> windows);

private:
	std::vector<inl::Window*> m_windows;
};
