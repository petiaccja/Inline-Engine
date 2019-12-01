#pragma once

namespace inl::game {

class LevelInputArchive;
class LevelOutputArchive;
class ComponentFactory;


class BasicLevel {
public:
	virtual void Load(LevelInputArchive& ar, ComponentFactory& factory) = 0;
	virtual void Save(LevelOutputArchive& ar, ComponentFactory& factory) const = 0;
};


} // namespace inl::game