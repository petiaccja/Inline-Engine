#include <InlineMath.hpp>
#include <cereal/cereal.hpp>


namespace mathter {

template <class Archive, class Scalar, int Dim, bool Packed>
void serialize(Archive& ar, Vector<Scalar, Dim, Packed>& obj) {
	for (int i = 0; i < obj.Dimension(); ++i) {
		ar(obj[i]);
	}
}


template <class Archive, class Scalar, int Rows, int Cols, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
void serialize(Archive& ar, Matrix<Scalar, Rows, Cols, Order, Layout, Packed>& obj) {
	for (int i = 0; i < obj.RowCount(); ++i) {
		for (int j = 0; j < obj.ColumnCount(); ++j) {
			ar(obj(i, j));
		}
	}
}


template <class Archive, class Scalar, bool Packed>
void serialize(Archive& ar, Quaternion<Scalar, Packed>& obj) {
	ar(obj.s);
}


} // namespace inl