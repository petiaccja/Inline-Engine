#pragma once


#include <InlineMath.hpp>

namespace inl {


/// <summary> Determines the way transformable entities calculate their velocity transforms. </summary>
enum class eMotionMode {
	EXPLICIT, /// <summary> Motion vectors explicitly set by the user. </summary>
	FIRST_ORDER, /// <summary> Motion vectors approximated from previous frame's transform. </summary>
	SECOND_ORDER, /// <summary> Motion vector approximated fromprevious two frames' transform. </summary>
};



/// <summary> Common interface for all transformable objects. </summary>
template <class T, int Dim, bool EnableMotion>
class ITransformable;

template <class T, int Dim>
class ITransformable23Base {
protected:
	using VectorT = Vector<T, Dim, false>;
	using MatLinT = Matrix<T, Dim, Dim, matrix_props::order, matrix_props::layout, false>;
	using MatHomT = Matrix<T, Dim+1, Dim+1, matrix_props::order, matrix_props::layout, false>;
	using RotT = std::conditional_t<Dim == 2, float, Quaternion<T, false>>;
public:
	virtual ~ITransformable23Base() = default;

	// Absolute transforms

	/// <summary> Linear position coordinates. </summary>
	virtual void SetPosition(const VectorT& pos) = 0;

	/// <summary> Set total rotation. </summary>
	/// <remarks> If shearing is present, the total rotation is the combination of the pre rotation and the post rotation.
	///		Only the post rotation is changed. </remarks>
	virtual void SetRotation(const RotT& rot) = 0;

	/// <summary> Sets the total scaling. </summary>
	/// <remarks> If shearing is present, the pre rotation and the post rotation are not changed,
	///		and the effect are visually illogical. </remarks>
	virtual void SetScale(const VectorT& scale) = 0;


	/// <summary> Sets a transformation matrix, without positioning. </summary>
	/// <remarks> Use it to add shear, scale and rotation in one go. </remarks>
	virtual void SetLinearTransform(const MatLinT& transform) = 0;

	/// <summary> Sets a transformation matrix, including positioning. </summary>
	virtual void SetTransform(const MatHomT& transform) = 0;


	// Get current transform

	/// <summary> Returns the current linear position coordinates. </summary>
	virtual const VectorT& GetPosition() const = 0;

	/// <summary> Returns the pre rotation which happens before scaling. </summary>
	virtual RotT GetShearRotation() const = 0;

	/// <summary> Returns the post rotation which happens after scaling. </summary>
	virtual RotT GetPostRotation() const = 0;

	/// <summary> Return the total rotation, which is the sum of the pre- and post rotations. </summary>
	virtual RotT GetRotation() const = 0;

	/// <summary> Return the current scale. </summary>
	/// <remarks> If shear is present, returns the scaling that happens between pre- and post rotations. </remarks>
	virtual const VectorT& GetScale() const = 0;


	/// <summary> Returns the transformation matrix, excluding translation. </summary>
	virtual MatLinT GetLinearTransform() const = 0;

	/// <summary> Returns the transformation matrix, including translation. </summary>
	virtual MatHomT GetTransform() const = 0;


	// Relative transforms

	/// <summary> Changes the position of the object by <paramref name="offset"/>. </summary>
	virtual void Move(const VectorT& offset) = 0;

	/// <summary> Applies given rotation to existing transform. </summary>
	/// <remarks> Regarless of shear, only the post rotation is changed. Existing shear is preserved. </remarks>
	virtual void Rotate(const RotT& rot) = 0;

	/// <summary> Applies scaling to existing transform. </summary>
	virtual void Scale(const VectorT& scale) = 0;

	/// <summary> Applies shear to existing transform. </summary>
	/// <remarks> All elements of the transform are changed, except position.
	///		Can be very expensive because of the SVD it has to do. </remarks>
	virtual void Shear(T slope, int primaryAxis, int perpAxis) = 0;
};


template <class T>
class ITransformable<T, 2, false> : public virtual ITransformable23Base<T, 2> {
protected:
	using typename ITransformable23Base<T, 2>::MatHomT;
public:
	virtual void ShearX(T slope) = 0;
	virtual void ShearY(T slope) = 0;
};


template <class T>
class ITransformable<T, 3, false> : public virtual ITransformable23Base<T, 3> {
protected:
	using typename ITransformable23Base<T, 3>::MatHomT;
public:
	virtual void ShearXY(T slope) = 0;
	virtual void ShearXZ(T slope) = 0;
	virtual void ShearYX(T slope) = 0;
	virtual void ShearYZ(T slope) = 0;
	virtual void ShearZX(T slope) = 0;
	virtual void ShearZY(T slope) = 0;
};


template <class T, int Dim>
class ITransformable<T, Dim, true> : public virtual ITransformable<T, Dim, false> {
protected:
	using typename ITransformable<T, Dim, false>::MatHomT;
public:
	/// <summary> Sets the matrix that transforms local points into world space motion vectors. </summary>
	virtual void SetTransformMotion(const MatHomT& motion) = 0;
	/// <summary> Gets the matrix that transforms local points into world space motion vectors.
	///		See <see cref="SetMotionMode"/> on how it is calculated. </summary>
	virtual MatHomT GetTransformMotion() const = 0;

	/// <summary> Called by the graphics engine every frame to update first order motion matrix. </summary>
	virtual void UpdateTransformMotion(float deltaTime) = 0;


	/// <summary> Determines how motion matrices are calculated. </summary>
	/// <remarks> Second order mode is not supported and treated as first order. </remarks>
	virtual void SetMotionMode(eMotionMode mode) = 0;
	virtual eMotionMode GetMotionMode() const = 0;
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
template <class T, int Dim, bool EnableMotion>
class Transformable;


template <class T, int Dim>
class Transformable23Base : public virtual ITransformable23Base<T, Dim> {
	static_assert(2 <= Dim && Dim <= 3, "This class is only a helper for 2D and 3D transforms.");
protected:
	using VectorT = Vector<T, Dim, false>;
	using MatLinT = Matrix<T, Dim, Dim, matrix_props::order, matrix_props::layout, false>;
	using MatHomT = Matrix<T, Dim+1, Dim+1, matrix_props::order, matrix_props::layout, false>;
	using RotT = std::conditional_t<Dim == 2, float, Quaternion<T, false>>;

public:
	Transformable23Base(const VectorT& scale = VectorT(1), const RotT& rotation = IdentityRot(), const VectorT& position = VectorT(0))
		: rotation1(IdentityRot()), scale(scale), rotation2(rotation), position(position)
	{}
	Transformable23Base(const MatLinT& linearTransform) {
		position.Spread(0);
		SetLinearTransform(linearTransform);
	}
	Transformable23Base(const MatHomT& fullTransform) {
		SetTransform(fullTransform);
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
	void SetLinearTransform(const MatLinT& transform);

	/// <summary> Sets a transformation matrix, including positioning. </summary>
	void SetTransform(const MatHomT& transform);


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
	MatLinT GetLinearTransform() const;

	/// <summary> Returns the transformation matrix, including translation. </summary>
	MatHomT GetTransform() const;


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


	// Motion matrix finite difference estimation
	static MatHomT MotionMatrix(MatHomT currentTransform, MatHomT pastTransform, T elapsed);
	static MatHomT MotionMatrix(MatHomT currentTransform, MatHomT pastTransform1, T elapsed1, MatHomT pastTransform2, T elapsed2);

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
class Transformable<T, 2, false> : public Transformable23Base<T, 2>, public virtual ITransformable<T, 2, false> {
protected:
	using typename Transformable23Base<T, 2>::MatHomT;
public:
	using Transformable23Base<T, 2>::Transformable23Base;

	void ShearX(T slope) { this->Shear(slope, 0, 1); }
	void ShearY(T slope) { this->Shear(slope, 1, 0); }
};


template <class T>
class Transformable<T, 3, false> : public Transformable23Base<T, 3>, public virtual ITransformable<T, 3, false> {
protected:
	using typename Transformable23Base<T, 3>::MatHomT;
public:
	using Transformable23Base<T, 3>::Transformable23Base;

	void ShearXY(T slope) { this->Shear(slope, 0, 1);	}
	void ShearXZ(T slope) { this->Shear(slope, 0, 2); }
	void ShearYX(T slope) { this->Shear(slope, 1, 0); }
	void ShearYZ(T slope) { this->Shear(slope, 1, 2); }
	void ShearZX(T slope) { this->Shear(slope, 2, 0); }
	void ShearZY(T slope) { this->Shear(slope, 2, 1); }
};


template <class T, int Dim>
class Transformable<T, Dim, true> : public Transformable<T, Dim, false>, public virtual ITransformable<T, Dim, true> {
protected:
	using typename Transformable<T, Dim, false>::MatHomT;
public:
	using Transformable<T, Dim, false>::Transformable;


	/// <summary> Sets the matrix that transforms local points into world space motion vectors. </summary>
	void SetTransformMotion(const MatHomT& motion);
	/// <summary> Gets the matrix that transforms local points into world space motion vectors.
	///		See <see cref="SetMotionMode"/> on how it is calculated. </summary>
	MatHomT GetTransformMotion() const;

	/// <summary> Called by the graphics engine every frame to update first order motion matrix. </summary>
	void UpdateTransformMotion(float deltaTime);


	/// <summary> Determines how motion matrices are calculated. </summary>
	/// <remarks> Second order mode is not supported and treated as first order. </remarks>
	void SetMotionMode(eMotionMode mode);
	eMotionMode GetMotionMode() const;

protected:
	eMotionMode m_motionMode = eMotionMode::FIRST_ORDER;
	MatHomT m_transformMotion = MatHomT::Identity(); // Either prev transform or explicit motion matrix.
	T m_deltaTime = T(1e+20); // Large value so that first motion is going to be zero anyway.
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
typename Transformable23Base<T, Dim>::MatLinT Transformable23Base<T, Dim>::GetLinearTransform() const {
	if constexpr (matrix_props::order == eMatrixOrder::FOLLOW_VECTOR) {
		return ToRotationMatrix(rotation1)*MatLinT::Scale(scale)*ToRotationMatrix(rotation2);
	}
	else {
		return ToRotationMatrix(rotation2)*MatLinT::Scale(scale)*ToRotationMatrix(rotation1);
	}
}

template <class T, int Dim>
typename Transformable23Base<T, Dim>::MatHomT Transformable23Base<T, Dim>::GetTransform() const {
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


template <class T, int Dim> 
typename Transformable23Base<T, Dim>::MatHomT Transformable23Base<T, Dim>::MotionMatrix(MatHomT currentTransform, MatHomT pastTransform, T elapsed) {
	// Calculate tap coefficients
	T c0 = 1/elapsed;
	T c1 = -c0;
	MatHomT motion = c0*currentTransform + c1*pastTransform;
	//motion(motion.RowCount()-1, motion.ColumnCount()-1) = 1;
	return motion;
}

template <class T, int Dim>
typename Transformable23Base<T, Dim>::MatHomT Transformable23Base<T, Dim>::MotionMatrix(MatHomT currentTransform, MatHomT pastTransform1, T elapsed1, MatHomT pastTransform2, T elapsed2) {
	// Calculate tap coefficients
	T c2 = elapsed1/(elapsed2*elapsed2 - elapsed1*elapsed2);
	T c1 = elapsed2/(elapsed1*elapsed1 - elapsed1*elapsed2);
	T c0 = -(c1 + c2);
	MatHomT motion = c0*currentTransform + c1*pastTransform1 + c2*pastTransform2;
	//motion(motion.RowCount()-1, motion.ColumnCount()-1) = 1;
	return motion;
}



//------------------------------------------------------------------------------
// Motion methods
//------------------------------------------------------------------------------
template <class T, int Dim>
void Transformable<T, Dim, true>::SetTransformMotion(const MatHomT& motion) {
	assert(m_motionMode == eMotionMode::EXPLICIT);
	m_transformMotion = motion;
}

template <class T, int Dim>
auto Transformable<T, Dim, true>::GetTransformMotion() const -> MatHomT {
	if (m_motionMode == eMotionMode::EXPLICIT) {
		return m_transformMotion;
	}
	else {
		return Transformable<T, Dim, false>::MotionMatrix(this->GetTransform(), m_transformMotion, m_deltaTime);
	}
}

template <class T, int Dim>
void Transformable<T, Dim, true>::UpdateTransformMotion(float deltaTime) {
	m_deltaTime = deltaTime;
	if (m_motionMode != eMotionMode::EXPLICIT) {
		m_transformMotion = this->GetTransform();
	}
}

template <class T, int Dim>
void Transformable<T, Dim, true>::SetMotionMode(eMotionMode mode) {
	m_motionMode = mode;
}

template <class T, int Dim>
eMotionMode Transformable<T, Dim, true>::GetMotionMode() const {
	return m_motionMode;
}


//------------------------------------------------------------------------------
// Typedefs
//------------------------------------------------------------------------------


/// <summary> 2D float transformable with motion. </summary>
using ITransformable2D = ITransformable<float, 2, true>;
/// <summary> 3D float transformable with motion. </summary>
using ITransformable3D = ITransformable<float, 3, true>;
/// <summary> 2D double transformable with motion. </summary>
using ITransformable2Dd = ITransformable<double, 2, true>;
/// <summary> 3D double transformable with motion. </summary>
using ITransformable3Dd = ITransformable<double, 3, true>;

/// <summary> 2D float transformable without motion. </summary>
using ITransformable2DN = ITransformable<float, 2, false>;
/// <summary> 3D float transformable without motion. </summary>
using ITransformable3DN = ITransformable<float, 3, false>;
/// <summary> 2D double transformable without motion. </summary>
using ITransformable2DNd = ITransformable<double, 2, false>;
/// <summary> 3D double transformable without motion. </summary>
using ITransformable3DNd = ITransformable<double, 3, false>;


/// <summary> 2D float transformable with motion. </summary>
using Transformable2D = Transformable<float, 2, true>;
/// <summary> 3D float transformable with motion. </summary>
using Transformable3D = Transformable<float, 3, true>;
/// <summary> 2D double transformable with motion. </summary>
using Transformable2Dd = Transformable<double, 2, true>;
/// <summary> 3D double transformable with motion. </summary>
using Transformable3Dd = Transformable<double, 3, true>;

/// <summary> 2D float transformable without motion. </summary>
using Transformable2DN = Transformable<float, 2, false>;
/// <summary> 3D float transformable without motion. </summary>
using Transformable3DN = Transformable<float, 3, false>;
/// <summary> 2D double transformable without motion. </summary>
using Transformable2DNd = Transformable<double, 2, false>;
/// <summary> 3D double transformable without motion. </summary>
using Transformable3DNd = Transformable<double, 3, false>;


} // namespace inl