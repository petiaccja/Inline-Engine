#pragma once


namespace inl::game {


class Entity;



class Component {
	friend class Entity;

public:
	const Entity* GetEntity() const {
		return m_entity;
	}

private:
	Entity* m_entity = nullptr;
};


} // namespace inl::game