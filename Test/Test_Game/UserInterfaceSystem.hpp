#pragma once

#include "ActionHeap.hpp"
#include "EngineCollection.hpp"
#include "UserInterfaceCompositor.hpp"

#include <GameLogic/System.hpp>
#include <GraphicsEngine/Resources/IFont.hpp>
#include <GraphicsEngine/Scene/ICamera2D.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>
#include <GuiEngine/Board.hpp>


namespace inl {
class Window;
} // namespace inl



class UserInterfaceSystem : public inl::game::System<UserInterfaceSystem> {
public:
	UserInterfaceSystem(const EngineCollection& modules, inl::Window& window, std::shared_ptr<ActionHeap> actionHeap);
	UserInterfaceSystem(UserInterfaceSystem&& rhs);
	UserInterfaceSystem(const UserInterfaceSystem&) = delete;
	UserInterfaceSystem& operator=(UserInterfaceSystem&&) = delete;
	UserInterfaceSystem& operator=(const UserInterfaceSystem&) = delete;
	~UserInterfaceSystem();

	void Update(float elapsed) override;
	UserInterfaceCompositor& GetCompositor();

private:
	void ResizeRender(inl::ResizeEvent evt);
	void KeyboardShortcuts(inl::KeyboardEvent evt);
	
	void RegisterBoardEvents();
	void UnregisterBoardEvents();
	void RegisterWindowEvents();
	void UnregisterWindowEvents();


private:
	std::unique_ptr<inl::gxeng::IFont> m_font;
	std::unique_ptr<inl::gxeng::IScene> m_scene;
	std::unique_ptr<inl::gxeng::ICamera2D> m_camera;
	std::unique_ptr<inl::gui::Board> m_board;
	std::unique_ptr<UserInterfaceCompositor> m_compositor;
	inl::Window& m_window;
	std::shared_ptr<ActionHeap> m_actionHeap;
};
