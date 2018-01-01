#pragma once


#include <InlineMath.hpp>

namespace inl {



template <class T, int Dim>
class Transformable;





template <class T, int Dim>
class Transformable23Base {
	static_assert(2 <= Dim && Dim <= 3, "This class is only a helper for 2D and 3D transforms.");

	using VectorT = Vector<T, Dim, false>;
	using MatLinT = Matrix<T, Dim, Dim, matrix_props::order, matrix_props::layout, false>;
	using MatHomT = Matrix<T, Dim+1, Dim+1, matrix_props::order, matrix_props::layout, false>;
	using RotT = std::conditional_t<Dim == 2, float, Quaternion<T, false>>;

public:
	Transformable23Base()
		: rotation1(IdentityRot()), scale(1), rotation2(IdentityRot()), position(0)
	{}

	// Absolute transforms
	void SetPosition(const VectorT& pos);
	void SetRotation(const RotT& rot);
	void SetScale(const VectorT& scale);

	void SetLinearTransform(const MatLinT& transform);
	void SetTransform(const MatHomT& transform);


	// Get current transform
	const VectorT& GetPosition() const { return position; }
	const RotT GetShearRotation() const { return rotation1; }
	const RotT GetRotation() const { return CombineRotations(rotation1, rotation2); }
	const VectorT& GetScale() const { return scale; }

	const MatLinT GetLinearTransform() const;
	const MatHomT GetTransform() const;


	// Relative transforms
	void Move(const VectorT& offset);
	void Rotate(const RotT& rot);
	void Scale(const VectorT& scale);

	void Shear(T slope, int primaryAxis, int perpAxis);
protected:
	// Helper functions
	static MatLinT ToRotationMatrix(const RotT& arg) {
		if constexpr (Dim == 2) {
			return MatLinT::Rotation(arg);
		}
		else {
			return (MatLinT)arg;
		}
	}
	static RotT FromRotationMatrix(const MatLinT& arg) {
		if constexpr (Dim == 2) {
			return atan2(matrix_props::order == eMatrixOrder::FOLLOW_VECTOR ? arg(0, 1) : arg(1, 0), arg(0, 0));
		}
		else {
			return (RotT)arg;
		}
	}
	static RotT CombineRotations(const RotT& rot1, const RotT& rot2) {
		if constexpr (Dim == 2) {
			return rot1 + rot2;
		}
		else {
			return rot2*rot1;
		}
	}
	static RotT InvertRotation(const RotT& arg) {
		if constexpr (Dim == 2) {
			return -arg;
		}
		else {
			return arg.Conjugate();
		}
	}
	static VectorT RotateVector(const VectorT& vec, const RotT& rot) {
		if constexpr (Dim == 2) {
			return vec*Mat22::Rotation(rot);
		}
		else {
			return vec*rot;
		}
	}
	static RotT IdentityRot() {
		if constexpr (Dim == 2) {
			return T(0);
		}
		else {
			return Quat::Identity();
		}	
	}
protected:
	// Linear transform: M = rot1*scale*rot2 or M = USV.
	// Values below are M's singular value decomposition compressed.
	RotT rotation1; // U matrix of SVD
	VectorT scale; // S singular values of the transform
	RotT rotation2; // V matrix of the SVD

	// Homogeneous transform.
	VectorT position;
};


template <class T>
class Transformable<T, 2> : public Transformable23Base<T, 2> {
public:
	void ShearX(T slope) { Shear(slope, 0, 1); }
	void ShearY(T slope) { Shear(slope, 1, 0); }
};


template <class T>
class Transformable<T, 3> : public Transformable23Base<T, 3> {
public:
	void ShearXY(T slope) { Shear(slope, 0, 1);	}
	void ShearXZ(T slope) { Shear(slope, 0, 2); }
	void ShearYX(T slope) { Shear(slope, 1, 0); }
	void ShearYZ(T slope) { Shear(slope, 1, 2); }
	void ShearZX(T slope) { Shear(slope, 2, 0); }
	void ShearZY(T slope) { Shear(slope, 2, 1); }
};


//------------------------------------------------------------------------------
// Transformable 2D-3D methods
//------------------------------------------------------------------------------
template <class T, int Dim>
void Transformable23Base<T, Dim>::SetPosition(const VectorT& pos) {
	position = pos;
}

template <class T, int Dim>
void Transformable23Base<T, Dim>::SetRotation(const RotT& rot) {
	rotation2 = CombineRotations(InvertRotation(rotation1), rot);
}

template <class T, int Dim>
void Transformable23Base<T, Dim>::SetScale(const VectorT& scale) {
	// If rotation1 is not identity, scale type-in does not make sense anyway.
	// We just change the singular values because that's the fastest.
	// Optionally we could reset rot1 and set rot2' = rot2*rot1 (quat mul).
	this->scale = scale;
}



template <class T, int Dim>
void Transformable23Base<T, Dim>::SetLinearTransform(const MatLinT& transform) {
	// Break matrix into rot1, scale and rot2 components via SVD.
	MatLinT U, S, V;
	transform.DecomposeSVD(U, S, V);
	MatLinT A = U*S*V;
	for (int i = 0; i<Dim; ++i) {
		scale(i) = S(i, i);
	}
	
	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		rotation1 = FromRotationMatrix(U);
		rotation2 = FromRotationMatrix(V);
	}
	else {
		rotation2 = FromRotationMatrix(U);
		rotation1 = FromRotationMatrix(V);
	}
}

template <class T, int Dim>
void Transformable23Base<T, Dim>::SetTransform(const MatHomT& transform) {
	// Extract position from matrix.
	auto Indexer = [&transform](int i, int j) {
		return matrix_props::order == eMatrixOrder::FOLLOW_VECTOR ? transform(i, j) : transform(j, i);
	};
	for (int i = 0; i<Dim; ++i) {
		position(i) = Indexer(Dim, i);
	}

	// Set linear part.
	SetLinearTransform(transform.Submatrix<Dim, Dim>(0, 0));
}


template <class T, int Dim>
const typename Transformable23Base<T, Dim>::MatLinT Transformable23Base<T, Dim>::GetLinearTransform() const {
	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		return ToRotationMatrix(rotation1)*MatLinT::Scale(scale)*ToRotationMatrix(rotation2);
	}
	else {
		return ToRotationMatrix(rotation2)*MatLinT::Scale(scale)*ToRotationMatrix(rotation1);
	}
}

template <class T, int Dim>
const typename Transformable23Base<T, Dim>::MatHomT Transformable23Base<T, Dim>::GetTransform() const {
	MatLinT linear = GetLinearTransform();
	MatHomT tr = MatHomT::Translation(position);
	tr.Submatrix<Dim, Dim>(0, 0) = linear;
	return tr;
}



template <class T, int Dim>
void Transformable23Base<T, Dim>::Move(const VectorT& offset) {
	position += offset;
}

template <class T, int Dim>
void Transformable23Base<T, Dim>::Rotate(const RotT& rot) {
	rotation2 = CombineRotations(rotation2, rot);
	position = RotateVector(position, rot);
}

template <class T, int Dim>
void Transformable23Base<T, Dim>::Scale(const VectorT& scale) {
	MatLinT linear = GetLinearTransform();
	MatLinT postScale = MatLinT::Scale(scale);

	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		SetLinearTransform(linear*postScale);
	}
	else {
		SetLinearTransform(postScale*linear);
	}
	position *= scale;
}

template <class T, int Dim>
void Transformable23Base<T, Dim>::Shear(T slope, int primaryAxis, int perpAxis) {
	MatLinT linear = GetLinearTransform();
	MatLinT postShear = MatLinT::Shear(slope, primaryAxis, perpAxis);

	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		SetLinearTransform(linear*postShear);
		position = position*postShear;
	}
	else {
		SetLinearTransform(postShear*linear);
		position = postShear*position;
	}
}


using Transformable2D = Transformable<float, 2>;
using Transformable3D = Transformable<float, 3>;
using Transformable2Dd = Transformable<double, 2>;
using Transformable3Dd = Transformable<double, 3>;


} // namespace inl