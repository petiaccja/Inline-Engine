#pragma once


#include <InlineMath.hpp>

namespace inl {



template <class T, int Dim>
class Transformable;


template <class T>
class Transformable<T, 3> {
	using VectorT = Vector<T, 3, false>;
	using QuatT = Quaternion<T, false>;
	using Mat33T = Matrix<T, 3, 3, matrix_props::order, matrix_props::layout, false>;
	using Mat44T = Matrix<T, 4, 4, matrix_props::order, matrix_props::layout, false>;
public:
	// Absolute transforms
	void SetPosition(const VectorT& pos) { position = pos; }
	void SetRotation(const QuatT& rot) { rotation = rot; }
	void SetScale(const VectorT& scale) { shearScale.SetScale(scale); }


	// Get current transform
	const VectorT& GetPosition() const { return position; }
	const QuatT& GetRotation() const { return rotation; }
	const VectorT GetScale() const { return VectorT{shearScale(0,0), shearScale(1,1), shearScale(2,2) }; }
	const Mat33T& GetShearScale() const { return shearScale; }

	const Mat44T GetTransform() const;
	

	// Relative transforms
	void Move(const VectorT& offset);
	void Rotate(const QuatT& rot);
	void Scale(const VectorT& scale);

	void Shear(T slope, int primaryAxis, int perpAxis);

	void ShearXY(T slope) { Shear(slope, 0, 1);	}
	void ShearXZ(T slope) { Shear(slope, 0, 2); }
	void ShearYX(T slope) { Shear(slope, 1, 0); }
	void ShearYZ(T slope) { Shear(slope, 1, 2); }
	void ShearZX(T slope) { Shear(slope, 2, 0); }
	void ShearZY(T slope) { Shear(slope, 2, 1); }
protected:
	Mat33T shearScale;
	QuatT rotation;
	VectorT position;
};



} // namespace inl