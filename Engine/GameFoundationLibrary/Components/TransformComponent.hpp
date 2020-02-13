#pragma once


#include <BaseLibrary/Transform.hpp>
#include <GameLogic/AutoRegisterComponent.hpp>
#include <BaseLibrary/Serialization/Math.hpp>


namespace inl::gamelib {


struct TransformComponent : Transform3D {
private:
	static constexpr char ClassName[] = "TransformComponent";
	inline static const game::AutoRegisterComponent<TransformComponent, ClassName> reg = {};
};



template <class Archive>
void save(Archive& ar, const TransformComponent& obj) {
	Mat44 matrix = obj.GetMatrix();
	ar(matrix);
}


template <class Archive>
void load(Archive& ar, TransformComponent& obj) {
	Mat44 matrix;
	ar(matrix);
	obj.SetMatrix(matrix);
}




} // namespace inl::gamelib