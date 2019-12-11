#pragma once

#include "ILevel.hpp"
#include "UserInterfaceCompositor.hpp"

#include <GuiEngine/Frame.hpp>


/// <summary> Captures user input that would not fall on the 3D scene, not the UI. </summary>
/// <remarks> It is a full-screen frame, that is placed behind all other UI controls.
///		Mouse clicks that miss other UI elements will hit this, and this control
///		can translate them to interactions with the 3D scene. </remarks>
class GameSceneFrame : public TopLevelFrame, public inl::gui::Frame {
public:
	GameSceneFrame();

	void SetGameWorld(Game& gameWorld);
	void Start(std::unique_ptr<ILevel> level);
private:
	Game* m_gameWorld = nullptr;
	std::unique_ptr<ILevel> m_level;
};
