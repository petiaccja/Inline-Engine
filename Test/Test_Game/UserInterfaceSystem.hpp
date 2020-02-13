#pragma once

#include "ActionHeap.hpp"
#include "ActionSystem.hpp"
#include "DebugInfoFrame.hpp"
#include "EngineCollection.hpp"
#include "GameSceneFrame.hpp"
#include "MainMenuFrame.hpp"
#include "PauseMenuFrame.hpp"
#include "WindowLayout.hpp"

#include <GameLogic/System.hpp>
#include <GraphicsEngine/Resources/IFont.hpp>
#include <GraphicsEngine/Scene/ICamera2D.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>
#include <GuiEngine/Board.hpp>


namespace inl {
class Window;
} // namespace inl



class UserInterfaceSystem : public inl::game::System<UserInterfaceSystem>, public ActionSystem {
	enum class eGameState {
		MAIN_MENU,
		LOADING,
		PLAYING,
		PAUSED,
		QUIT,
	};

public:
	UserInterfaceSystem(const EngineCollection& modules, inl::Window& window);
	UserInterfaceSystem(UserInterfaceSystem&& rhs);
	UserInterfaceSystem(const UserInterfaceSystem&) = delete;
	UserInterfaceSystem& operator=(UserInterfaceSystem&&) = delete;
	UserInterfaceSystem& operator=(const UserInterfaceSystem&) = delete;
	~UserInterfaceSystem();

	void ReactActions(ActionHeap& actions) override;
	void Update(float elapsed) override;
	void EmitActions(ActionHeap& actions) override;

private:
	void ResizeRender(inl::ResizeEvent evt);
	void KeyboardShortcuts(inl::KeyboardEvent evt);

	void RegisterBoardEvents();
	void UnregisterBoardEvents();
	void RegisterWindowEvents();
	void UnregisterWindowEvents();

	void CreateFrames();
	void RegisterHandlers();
	void UnregisterHandlers();

	static std::filesystem::path GetSaveFileDir();
	
private:
	std::shared_ptr<inl::gxeng::IFont> m_font;
	std::unique_ptr<inl::gxeng::IScene> m_scene;
	std::unique_ptr<inl::gxeng::ICamera2D> m_camera;
	std::unique_ptr<inl::gui::Board> m_board;
	inl::Window& m_window;
	struct Controls {
		std::shared_ptr<WindowLayout> layout;
		std::shared_ptr<DebugInfoFrame> debugInfo;
		std::shared_ptr<MainMenuFrame> mainMenu;
		std::shared_ptr<PauseMenuFrame> pauseMenu;
		std::shared_ptr<GameSceneFrame> background;
	} m_controls;
	std::optional<std::reference_wrapper<ActionHeap>> m_transientActionHeap;
	eGameState m_gameState = eGameState::MAIN_MENU;
};
