#include <GameLogic/AutoRegisterComponent.hpp>
#include <GameLogic/ComponentClassFactory.hpp>

#include <cereal/cereal.hpp>


class FooComponent {
public:
	float value = 0.0f;

private:
	static constexpr char ClassName[] = "FooComponent";
	static constexpr inl::game::AutoRegisterComponent<FooComponent, ClassName> reg{};
};

class BarComponent {
public:
	float value = 1.0f;

private:
	static constexpr char ClassName[] = "BarComponent";
	static constexpr inl::game::AutoRegisterComponent<BarComponent, ClassName> reg{};
};

class BazComponent {
public:
	float value = 2.0f;

private:
	static constexpr char ClassName[] = "BazComponent";
	static constexpr inl::game::AutoRegisterComponent<BazComponent, ClassName> reg{};
};


class SpecialFactory : public inl::game::ComponentClassFactoryBase {
public:
	void Create(inl::game::Entity& entity) override;
	void Create(inl::game::Entity& entity, inl::game::InputArchive& archive) override;
	void Configure(float defvalue) {
		m_defvalue = defvalue;
	}
	std::unique_ptr<ComponentClassFactoryBase> Clone() override;

private:
	float m_defvalue = 0.0f;
};


class SpecialComponent {
public:
	float value = 3.0f;
	static constexpr char ClassName[] = "SpecialComponent";
	static constexpr inl::game::AutoRegisterComponent<SpecialComponent, ClassName, SpecialFactory> reg{};
};


inline void SpecialFactory::Create(inl::game::Entity& entity) {
	entity.AddComponent(SpecialComponent{ m_defvalue });
}


inline void SpecialFactory::Create(inl::game::Entity& entity, inl::game::InputArchive& archive) {
	SpecialComponent component{};
	archive(component);
	entity.AddComponent(std::move(component));
}

inline std::unique_ptr<inl::game::ComponentClassFactoryBase> SpecialFactory::Clone() {
	return std::make_unique<SpecialFactory>(*this);
}


template <class Archive>
void serialize(Archive& ar, FooComponent& obj) {
	ar(obj.value);
}

template <class Archive>
void serialize(Archive& ar, BarComponent& obj) {
	ar(obj.value);
}

template <class Archive>
void serialize(Archive& ar, BazComponent& obj) {
	ar(obj.value);
}

template <class Archive>
void serialize(Archive& ar, SpecialComponent& obj) {
	ar(obj.value);
}