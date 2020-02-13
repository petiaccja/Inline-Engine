#pragma once


#include <InlineMath.hpp>

namespace inl {


/// <summary> Determines the way transformable entities calculate their velocity transforms. </summary>
enum class eMotionMode {
	EXPLICIT, /// <summary> Motion vectors explicitly set by the user. </summary>
	FIRST_ORDER, /// <summary> Motion vectors approximated from previous frame's transform. </summary>
	SECOND_ORDER, /// <summary> Motion vector approximated fromprevious two frames' transform. </summary>
};


/// <summary> Stores 2D or 3D affine transforms. </summary>
/// <remarks> Any affine transform can be represented, including translation, rotation, scaling
///		and shear.
///		<para/>
///		All affine transforms can be represented as the succession of a rotation, a scaling,
///		another rotation and a translation (RSRT). Translation can be treated separately.
///		The RSR sequence can be represented with a single square matrix M (3x3 for 3D), however,
///		using singular value decomposition, M can be decomposed as M = USV. Here, U and V are rotation
///		matrices and S is a scaling matrix. As such, U and V can be represented by quaternions
///		and are called pre/shear- and post rotation. S can simply be represented by a vector.
///		<para/>
///		The quat-vec3-quat-vec3 representation is used internally for fast setting of these values.
///		These fields can be queried individually, too.
///		This also results in applying scaling and shear as relative transforms being slow, because
///		an SVD needs to be performed. Querying the total transform matrix is acceptable,
///		but still has a cost.
///		</remarks>
template <class T, int Dim>
class Transform;


template <class T, int Dim>
class Transform23Base  {
	static_assert(2 <= Dim && Dim <= 3, "This class is only a helper for 2D and 3D transforms.");

protected:
	using VectorT = Vector<T, Dim, false>;
	using MatLinT = Matrix<T, Dim, Dim, matrix_props::order, matrix_props::layout, false>;
	using MatHomT = Matrix<T, Dim + 1, Dim + 1, matrix_props::order, matrix_props::layout, false>;
	using RotT = std::conditional_t<Dim == 2, float, Quaternion<T, false>>;

public:
	Transform23Base(const VectorT& scale = VectorT(1), const RotT& rotation = IdentityRot(), const VectorT& position = VectorT(0))
		: rotation1(IdentityRot()), scale(scale), rotation2(rotation), position(position) {}
	Transform23Base(const MatLinT& linearTransform) {
		position.Spread(0);
		SetLinearMatrix(linearTransform);
	}
	Transform23Base(const MatHomT& fullTransform) {
		SetMatrix(fullTransform);
	}

	// Absolute transforms

	/// <summary> Linear position coordinates. </summary>
	void SetPosition(const VectorT& pos);

	/// <summary> Set total rotation. </summary>
	/// <remarks> If shearing is present, the total rotation is the combination of the pre rotation and the post rotation.
	///		Only the post rotation is changed. </remarks>
	void SetRotation(const RotT& rot);

	/// <summary> Sets the total scaling. </summary>
	/// <remarks> If shearing is present, the pre rotation and the post rotation are not changed,
	///		and the effect are visually illogical. </remarks>
	void SetScale(const VectorT& scale);


	/// <summary> Sets a transformation matrix, without positioning. </summary>
	/// <remarks> Use it to add shear, scale and rotation in one go. </remarks>
	void SetLinearMatrix(const MatLinT& transform);

	/// <summary> Sets a transformation matrix, including positioning. </summary>
	void SetMatrix(const MatHomT& transform);


	// Get current transform

	/// <summary> Returns the current linear position coordinates. </summary>
	const VectorT& GetPosition() const { return position; }

	/// <summary> Returns the pre rotation which happens before scaling. </summary>
	RotT GetShearRotation() const { return rotation1; }

	/// <summary> Returns the post rotation which happens after scaling. </summary>
	RotT GetPostRotation() const { return rotation2; }

	/// <summary> Return the total rotation, which is the sum of the pre- and post rotations. </summary>
	RotT GetRotation() const { return CombineRotations(rotation1, rotation2); }

	/// <summary> Return the current scale. </summary>
	/// <remarks> If shear is present, returns the scaling that happens between pre- and post rotations. </remarks>
	const VectorT& GetScale() const { return scale; }


	/// <summary> Returns the transformation matrix, excluding translation. </summary>
	MatLinT GetLinearMatrix() const;

	/// <summary> Returns the transformation matrix, including translation. </summary>
	MatHomT GetMatrix() const;


	// Relative transforms

	/// <summary> Changes the position of the object by <paramref name="offset"/>. </summary>
	void Move(const VectorT& offset);

	/// <summary> Applies given rotation to existing transform. </summary>
	/// <remarks> Regarless of shear, only the post rotation is changed. Existing shear is preserved. </remarks>
	void Rotate(const RotT& rot);

	/// <summary> Applies scaling to existing transform. </summary>
	void Scale(const VectorT& scale);

	/// <summary> Applies shear to existing transform. </summary>
	/// <remarks> All elements of the transform are changed, except position.
	///		Can be very expensive because of the SVD it has to do. </remarks>
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
			return rot2 * rot1;
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
			return vec * Mat22::Rotation(rot);
		}
		else {
			return vec * rot;
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
class Transform<T, 2> : public Transform23Base<T, 2> {
protected:
	using typename Transform23Base<T, 2>::MatHomT;

public:
	using Transform23Base<T, 2>::Transform23Base;

	void ShearX(T slope) { this->Shear(slope, 0, 1); }
	void ShearY(T slope) { this->Shear(slope, 1, 0); }
};


template <class T>
class Transform<T, 3> : public Transform23Base<T, 3> {
protected:
	using typename Transform23Base<T, 3>::MatHomT;

public:
	using Transform23Base<T, 3>::Transform23Base;

	void ShearXY(T slope) { this->Shear(slope, 0, 1); }
	void ShearXZ(T slope) { this->Shear(slope, 0, 2); }
	void ShearYX(T slope) { this->Shear(slope, 1, 0); }
	void ShearYZ(T slope) { this->Shear(slope, 1, 2); }
	void ShearZX(T slope) { this->Shear(slope, 2, 0); }
	void ShearZY(T slope) { this->Shear(slope, 2, 1); }
};


//------------------------------------------------------------------------------
// Transformable 2D-3D methods
//------------------------------------------------------------------------------
template <class T, int Dim>
void Transform23Base<T, Dim>::SetPosition(const VectorT& pos) {
	position = pos;
}

template <class T, int Dim>
void Transform23Base<T, Dim>::SetRotation(const RotT& rot) {
	rotation2 = CombineRotations(InvertRotation(rotation1), rot);
}

template <class T, int Dim>
void Transform23Base<T, Dim>::SetScale(const VectorT& scale) {
	// If rotation1 is not identity, scale type-in does not make sense anyway.
	// We just change the singular values because that's the fastest.
	// Optionally we could reset rot1 and set rot2' = rot2*rot1 (quat mul).
	this->scale = scale;
}



template <class T, int Dim>
void Transform23Base<T, Dim>::SetLinearMatrix(const MatLinT& transform) {
	// Break matrix into rot1, scale and rot2 components via SVD.
	MatLinT U, S, V;
	transform.DecomposeSVD(U, S, V);
	MatLinT A = U * S * V;
	for (int i = 0; i < Dim; ++i) {
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
void Transform23Base<T, Dim>::SetMatrix(const MatHomT& transform) {
	// Extract position from matrix.
	auto Indexer = [&transform](int i, int j) {
		return matrix_props::order == eMatrixOrder::FOLLOW_VECTOR ? transform(i, j) : transform(j, i);
	};
	for (int i = 0; i < Dim; ++i) {
		position(i) = Indexer(Dim, i);
	}

	// Set linear part.
	SetLinearMatrix(transform.Submatrix<Dim, Dim>(0, 0));
}


template <class T, int Dim>
typename Transform23Base<T, Dim>::MatLinT Transform23Base<T, Dim>::GetLinearMatrix() const {
	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		return ToRotationMatrix(rotation1) * MatLinT::Scale(scale) * ToRotationMatrix(rotation2);
	}
	else {
		return ToRotationMatrix(rotation2) * MatLinT::Scale(scale) * ToRotationMatrix(rotation1);
	}
}

template <class T, int Dim>
typename Transform23Base<T, Dim>::MatHomT Transform23Base<T, Dim>::GetMatrix() const {
	MatLinT linear = GetLinearMatrix();
	MatHomT tr = MatHomT::Translation(position);
	tr.Submatrix<Dim, Dim>(0, 0) = linear;
	return tr;
}



template <class T, int Dim>
void Transform23Base<T, Dim>::Move(const VectorT& offset) {
	position += offset;
}

template <class T, int Dim>
void Transform23Base<T, Dim>::Rotate(const RotT& rot) {
	rotation2 = CombineRotations(rotation2, rot);
	position = RotateVector(position, rot);
}

template <class T, int Dim>
void Transform23Base<T, Dim>::Scale(const VectorT& scale) {
	MatLinT linear = GetLinearMatrix();
	MatLinT postScale = MatLinT::Scale(scale);

	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		SetLinearMatrix(linear * postScale);
	}
	else {
		SetLinearMatrix(postScale * linear);
	}
	position *= scale;
}

template <class T, int Dim>
void Transform23Base<T, Dim>::Shear(T slope, int primaryAxis, int perpAxis) {
	MatLinT linear = GetLinearMatrix();
	MatLinT postShear = MatLinT::Shear(slope, primaryAxis, perpAxis);

	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		SetLinearMatrix(linear * postShear);
		position = position * postShear;
	}
	else {
		SetLinearMatrix(postShear * linear);
		position = postShear * position;
	}
}


//------------------------------------------------------------------------------
// Typedefs
//------------------------------------------------------------------------------

/// <summary> 2D float transformable without motion. </summary>
using Transform2D = Transform<float, 2>;
/// <summary> 3D float transformable without motion. </summary>
using Transform3D = Transform<float, 3>;
/// <summary> 2D double transformable without motion. </summary>
using Transform2Dd = Transform<double, 2>;
/// <summary> 3D double transformable without motion. </summary>
using Transform3Dd = Transform<double, 3>;


} // namespace inl