#pragma once

#include "MeshBuffer.hpp"
#include "Vertex.hpp"

#include <type_traits>


namespace inl {
namespace gxeng {


class Mesh : protected MeshBuffer {
public:
	struct Element {
		eVertexElementSemantic semantic;
		int index;
		int offset;
	};
	struct Layout {
	public:
		Layout() = default;
		Layout(std::vector<std::vector<Element>> layout) : m_layout(std::move(layout)) {
			CalculateHashes(m_layout, m_elementHash, m_layoutHash);
		}
		const std::vector<Element>& operator[](size_t idx) const { return m_layout[idx]; }

		bool EqualElements(const Layout& rhs) const;
		bool EqualLayout(const Layout& rhs) const;
		size_t GetElementHash() const;
		size_t GetLayoutHash() const;
		size_t GetStreamCount() const;

		void Clear() { m_layout.clear(); m_elementHash = m_layoutHash = 0; }

	private:
		static void CalculateHashes(const std::vector<std::vector<Element>>& layout, size_t& elementHash, size_t& layoutHash);
		static std::vector<Element> GetAllElements(const std::vector<std::vector<Element>>& layout);
		static void RadixSortElements(std::vector<Element>& elements);

	private:
		std::vector<std::vector<Element>> m_layout;
		size_t m_elementHash = 0;
		size_t m_layoutHash = 0;
	};
public:
	Mesh(MemoryManager* memoryManager) : MeshBuffer(memoryManager) {}

	void Set(const VertexBase* vertices, const IVertexReader* vertexReader, size_t numVertices, const unsigned* indices, size_t numIndices);
	void Update(const VertexBase* vertices, const IVertexReader* vertexReader, size_t numVertices, size_t offsetInVertices);
	void Clear();

	using MeshBuffer::GetNumStreams;
	using MeshBuffer::GetVertexBuffer;
	using MeshBuffer::GetVertexBufferStride;
	using MeshBuffer::GetIndexBuffer;
	using MeshBuffer::IsIndexBuffer32Bit;

	const Layout& GetLayout() const;
private:
	Layout m_layout;
};



} // namespace gxeng
} // namespace inl