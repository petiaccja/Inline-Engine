#pragma once


#include <BaseLibrary/Transformable.hpp>
#include <GameLogic/AutoRegisterComponent.hpp>
#include <BaseLibrary/Serialization/Math.hpp>


namespace inl::gamelib {


struct TransformComponent : Transformable3DN {
private:
	static constexpr char ClassName[] = "TransformComponent";
	inline static const game::AutoRegisterComponent<TransformComponent, ClassName> reg = {};
};



template <class Archive>
void save(Archive& ar, const TransformComponent& obj) {
	Mat44 matrix = obj.GetTransform();
	ar(matrix);
}


template <class Archive>
void load(Archive& ar, TransformComponent& obj) {
	Mat44 matrix;
	ar(matrix);
	obj.SetTransform(matrix);
}




} // namespace inl::gamelib