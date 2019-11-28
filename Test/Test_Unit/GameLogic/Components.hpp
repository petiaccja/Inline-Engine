#pragma once

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


template <class Archive>
void save(Archive& ar, const FooComponent& obj) {
	ar(obj.value);
}
template <class Archive>
void load(Archive& ar, FooComponent& obj) {
	ar(obj.value);
}

template <class Archive>
void save(Archive& ar, const BarComponent& obj) {
	ar(obj.value);
}
template <class Archive>
void load(Archive& ar, BarComponent& obj) {
	ar(obj.value);
}

template <class Archive>
void save(Archive& ar, const BazComponent& obj) {
	ar(obj.value);
}
template <class Archive>
void load(Archive& ar, BazComponent& obj) {
	ar(obj.value);
}