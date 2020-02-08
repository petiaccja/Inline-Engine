#pragma once

#include <GameLogic/System.hpp>


namespace inl::game {
class LevelInputArchive;
class ComponentFactory;
class LevelOutputArchive;
} // namespace inl::game

class LevelSystem : public inl::game::SpecificSystem<LevelSystem> {
public:
	void Update(float elapsed) override;
	void Modify(const inl::game::Scene&);

private:
	void Load(inl::game::LevelInputArchive& ar, inl::game::ComponentFactory& factory);
	void Save(inl::game::LevelOutputArchive& ar, inl::game::ComponentFactory& factory) const;
	void Clear();

private:
};
