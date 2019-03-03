#pragma once


namespace inl::game {


class GameEntity;



class Component {
	friend class GameEntity;
public:

private:
	GameEntity* m_entity = nullptr;
};


} // namespace inl::game