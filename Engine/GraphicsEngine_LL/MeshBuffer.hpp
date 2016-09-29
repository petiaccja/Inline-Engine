#pragma once

#include <vector>
#include <memory>
#include <cstdint>

#include "GpuBuffer.hpp"


namespace inl {
namespace gxeng {


class MemoryManager;


//class VertexStream {
//public:
//	VertexStream();
//	VertexStream(const VertexStream&);
//	VertexStream(VertexStream&&);
//	VertexStream& operator=(const VertexStream&);
//	VertexStream& operator=(VertexStream&&);
//	~VertexStream() = default;
//
//	/// <summary> Initializes stream to the specified size, with garbage initial data. </summary>
//	/// <param name="stride"> Size of one vertex in bytes. </summary>
//	/// <param name="count"> Number of vertices. </summary>
//	void Set(int stride, int count);
//
//	/// <summary> Initializes stream to the specified size, copying 'data' </summary>
//	/// <param name="data"> Stream is filled with memory pointed by data. Must be at least stride*count bytes. </summary>
//	/// <param name="stride"> Size of one vertex in bytes. </summary>
//	/// <param name="count"> Number of vertices. </summary>
//	void Set(const void* data, int stride, int count);
//
//	/// <summary> Initializes stream to the specified size, taking ownership of 'data' </summary>
//	/// <param name="data"> Stream is filled with data. Must be at least stride*count bytes. Data becomes invalid. </summary>
//	/// <param name="stride"> Size of one vertex in bytes. </summary>
//	/// <param name="count"> Number of vertices. </summary>
//	void Set(std::unique_ptr<uint8_t>&& data, int stride, int count);
//	void Clear();
//	bool IsEmpty() const;
//	
//	void* Data() { return m_data.get(); }
//	const void* Data() const { return m_data.get(); }
//
//	int VertexCount() const { return m_count; }
//	int VertexStride() const { return m_stride; }
//	int Size() const { return m_count * m_stride; }
//private:
//	std::unique_ptr<uint8_t> m_data;
//	int m_stride;
//	int m_count;
//};

struct VertexStream {
	void* data;
	uint32_t stride;
	size_t count;
};


class MeshBuffer {
	enum class eValidationResult {
		OK,
		CLEAR,
		VERTEX_COUNT_MISMATCH,
		INDEX_TOO_LARGE,
		NOT_TRIANGLE,
	};
public:
	MeshBuffer(MemoryManager* memoryManager);

	template <class StreamIt, class IndexIt>
	void Set(StreamIt firstStream, StreamIt lastStream, IndexIt firstIndex, IndexIt lastIndex);

	void Update(uint32_t streamIndex, const void* vertexData, size_t vertexCount, size_t offsetInVertex);
	void Clear();

	size_t GetNumStreams() const;
	const std::shared_ptr<const VertexBuffer>& GetVertexBuffer(size_t streamIndex) const;
	size_t GetVertexBufferStride(size_t streamIndex) const;
	const std::shared_ptr<const IndexBuffer>& GetIndexBuffer() const;
private:
	template <class StreamIt, class IndexIt>
	eValidationResult Validate(StreamIt firstStream, StreamIt lastStream, IndexIt firstIndex, IndexIt lastIndex);
private:
	std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
	std::vector<size_t> m_vertexStrides;
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	MemoryManager* m_memoryManager;
};



} // namespace gxeng
} // namespace inl
