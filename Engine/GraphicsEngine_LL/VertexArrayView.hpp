#include "Vertex.hpp"
#include <type_traits>


namespace inl {
namespace gxeng {



template <class ViewT>
class VertexArrayView {
	static constexpr bool IsConst = std::is_const<ViewT>::value;
public:
	VertexArrayView();
	VertexArrayView(const VertexArrayView&) = default;
	VertexArrayView(VertexArrayView&&) = default;
	~VertexArrayView() = default;

	template <class VertexT, class = std::enable_if_t<IsConst>>
	VertexArrayView(const VertexT* array, size_t size) {
		Init(array, size);
	}

	template <class VertexT, class = std::enable_if_t<!IsConst>>
	VertexArrayView(VertexT* array, size_t size) {
		static_assert(!std::is_const_v<VertexT>, "You cannot initalize a mutable view with a const array.");
		Init(array, size);
	}

	void Clear() {
		m_array = nullptr;
		m_stride = m_size = 0;
	}

	
	template<class = std::enable_if_t<!IsConst, const int>>
	ViewT& operator[](size_t idx) {
		assert(idx < m_size);
		return *reinterpret_cast<ViewT*>(size_t(m_array) + idx*m_stride);
	}

	ViewT& operator[](size_t idx) const {
		assert(idx < m_size);
		return *reinterpret_cast<ViewT*>(size_t(m_array) + idx*m_stride);
	}
	
private:
	template <class PVertexT>
	void Init(PVertexT array, size_t size) {
		m_stride = sizeof(typename std::remove_pointer<PVertexT>::type);
		m_array = dynamic_cast<ViewT*>(array);
		m_size = size;
	}


	ViewT* m_array;
	size_t m_stride;
	size_t m_size;
};



}
}
