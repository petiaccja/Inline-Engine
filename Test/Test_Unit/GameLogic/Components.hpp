#include <GameLogic/AutoRegisterComponent.hpp>
#include <GameLogic/ComponentClassFactory.hpp>


class FooComponent {
public:
	float value = 0.0f;
	static constexpr char ClassName[] = "FooComponent";
	static constexpr inl::game::AutoRegisterComponent<FooComponent> reg{};
};

class BarComponent {
public:
	float value = 1.0f;
	static constexpr char ClassName[] = "BarComponent";
	static constexpr inl::game::AutoRegisterComponent<BarComponent> reg{};
};

class BazComponent {
public:
	float value = 2.0f;
	static constexpr char ClassName[] = "BazComponent";
	static constexpr inl::game::AutoRegisterComponent<BazComponent> reg{};
};


class SpecialFactory : public inl::game::ComponentClassFactoryBase {
public:
	void Create(inl::game::Entity& entity) override;
	void Configure(float defvalue) {
		m_defvalue = defvalue;
	}
private:
	float m_defvalue = 0.0f;
};


class SpecialComponent {
public:
	float value = 3.0f;
	static constexpr char ClassName[] = "SpecialComponent";
	static constexpr inl::game::AutoRegisterComponent<SpecialComponent, SpecialFactory> reg{};
};


inline void SpecialFactory::Create(inl::game::Entity& entity) {
	entity.AddComponent(SpecialComponent{ m_defvalue });
}
