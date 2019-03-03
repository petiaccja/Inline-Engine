#pragma once


namespace inl::game {


class GameEntity;



class Component {
	friend class GameEntity;

public:
	const GameEntity* GetEntity() const {
		return m_entity;
	}

private:
	GameEntity* m_entity = nullptr;
};


} // namespace inl::game