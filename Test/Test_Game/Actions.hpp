#pragma once

#include <GameLogic/BasicLevel.hpp>
#include <GameLogic/Scene.hpp>

#include <functional>
#include <memory>
#include <string>



struct ResizeScreenAction {
	unsigned width;
	unsigned height;
};


struct EnableRenderingAction {
	std::string cameraName;
};


struct DisableRenderingAction {};


struct LoadLevelAction {
public:
	LoadLevelAction(std::function<std::unique_ptr<inl::game::BasicLevel>(inl::game::Scene&)> creator) : m_creator(creator) {}
	[[nodiscard]] std::unique_ptr<inl::game::BasicLevel> Level(inl::game::Scene& scene) {
		if (!m_creator) {
			throw inl::InvalidStateException{};
		}
		auto result = m_creator(scene);
		m_creator = {};
		return result;
	}

private:
	std::function<std::unique_ptr<inl::game::BasicLevel>(inl::game::Scene&)> m_creator;
};


struct UpdateLoadingAction {
	std::string currentTask;
	std::string currentResource;
	float percentage;
	bool finished;
};