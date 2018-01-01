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
	void SetPosition(const VectorT& pos);
	void SetRotation(const QuatT& rot);
	void SetScale(const VectorT& scale);

	void SetLinearTransform(const Mat33T& transform);
	void SetTransform(const Mat44T& transform);


	// Get current transform
	const VectorT& GetPosition() const { return position; }
	const QuatT& GetRotation() const { return rotation2*rotation1; }
	const VectorT GetScale() const { return scale; }

	const Mat33T GetLinearTransform() const;
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
	// Linear transform: M = rot1*scale*rot2 or M = USV.
	// Values below are M's singular value decomposition compressed.
	QuatT rotation1; // U matrix of SVD
	VectorT scale; // S singular values of the transform
	QuatT rotation2; // V matrix of the SVD

	// Homogeneous transform.
	VectorT position;
};


//------------------------------------------------------------------------------
// Transformable 3D methods
//------------------------------------------------------------------------------
template <class T>
void Transformable<T, 3>::SetPosition(const VectorT& pos) {
	position = pos;
}

template <class T>
void Transformable<T, 3>::SetRotation(const QuatT& rot) {
	// rot2*rot1 = newrot
	rotation2 = rot*rotation1.Inverse();
}

template <class T>
void Transformable<T, 3>::SetScale(const VectorT& scale) {
	// If rotation1 is not identity, scale type-in does not make sense anyway.
	// We just change the singular values because that's the fastest.
	// Optionally we could reset rot1 and set rot2' = rot2*rot1 (quat mul).
	this->scale = scale;
}



template <class T>
void Transformable<T, 3>::SetLinearTransform(const Mat33T& transform) {
	// Break matrix into rot1, scale and rot2 components via SVD.
	Mat33T U, S, V;
	transform.DecomposeSVD(U, S, V);
	scale = { S(0,0), S(1,1), S(2,2) };
	
	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		rotation1 = U;
		rotation2 = V;
	}
	else {
		rotation2 = U;
		rotation1 = V;
	}
}

template <class T>
void Transformable<T, 3>::SetTransform(const Mat44T& transform) {
	// Extract position from matrix.
	auto Indexer = [&transform](int i, int j) {
		return matrix_props::order == eMatrixOrder::FOLLOW_VECTOR ? transform(i, j) : transform(j, i);
	};
	position(0) = Indexer(3, 0);
	position(1) = Indexer(3, 1);
	position(2) = Indexer(3, 2);

	// Set linear part.
	SetLinearTransform(transform.Submatrix<3, 3>(0, 0));
}


template <class T>
const Transformable<T, 3>::Mat33T Transformable<T, 3>::GetLinearTransform() const {
	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		return Mat33T(rotation1)*Mat33T::Scale(scale)*Mat33T(rotation2);
	}
	else {
		return Mat33T(rotation2)*Mat33T::Scale(scale)*Mat33T(rotation1);
	}
}

template <class T>
const Transformable<T, 3>::Mat44T Transformable<T, 3>::GetTransform() const {
	Mat33T linear = GetLinearTransform();
	Mat44T tr = Mat44T::Translation(position);
	tr.Submatrix<3, 3>(0, 0) = linear;
	return tr;
}



template <class T>
void Transformable<T, 3>::Move(const VectorT& offset) {
	position += offset;
}

template <class T>
void Transformable<T, 3>::Rotate(const QuatT& rot) {
	rotation2 = rotation2*rot;
}

template <class T>
void Transformable<T, 3>::Scale(const VectorT& scale) {
	Mat33T linear = GetLinearTransform();
	Mat33T postScale = Mat33T::Scale(scale);

	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		SetLinearTransform(linear*postScale);
	}
	else {
		SetLinearTransform(postScale*linear);
	}
}

template <class T>
void Transformable<T, 3>::Shear(T slope, int primaryAxis, int perpAxis) {
	Mat33T linear = GetLinearTransform();
	Mat33T postShear = Mat33T::Shear(slope, primaryAxis, perpAxis);

	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		SetLinearTransform(linear*postShear);
	}
	else {
		SetLinearTransform(postShear*linear);
	}
}


} // namespace inl