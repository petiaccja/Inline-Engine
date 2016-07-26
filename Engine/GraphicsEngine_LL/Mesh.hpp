#pragma once

#include <vector>
#include <memory>
#include <cstdint>

#include "GpuBuffer.hpp"


namespace inl {
namespace gxeng {



class VertexStream {
public:
	VertexStream();
	VertexStream(const VertexStream&);
	VertexStream(VertexStream&&);
	VertexStream& operator=(const VertexStream&);
	VertexStream& operator=(VertexStream&&);
	~VertexStream() = default;

	void Set(int stride, int count);
	void Set(const void* data, int stride, int count);
	void Set(std::unique_ptr<uint8_t>&& data, int stride, int count);
	void Clear();
	bool IsEmpty() const;
	
	void* Data() { return m_data.get(); }
	const void* Data() const { return m_data.get(); }

	int VertexCount() const { return m_count; }
	int VertexStride() const { return m_stride; }
	int Size() const { return m_count * m_stride; }
private:
	std::unique_ptr<uint8_t> m_data;
	int m_stride;
	int m_count;
};


class Mesh {
	enum class eValidationResult {
		OK,
		CLEAR,
		VERTEX_COUNT_MISMATCH,
		INDEX_TOO_LARGE,
		NOT_TRIANGLE,
	};
public:
	void Set(std::vector<VertexStream> streams, std::vector<unsigned> indices);
	void Update(int streamIndex, const void* vertexData, int vertexCount, int offsetInVertex);
	void Clear();

	int GetNumSreams() const;
	const VertexBuffer* GetVertexBuffer(int streamIndex) const;
	const IndexBuffer* GetIndexBuffer() const;
private:
	eValidationResult Validate(const std::vector<VertexStream>& streams, std::vector<unsigned> indices);
	void Optimize(std::vector<VertexStream>& streams, std::vector<unsigned>& indices);
private:
	std::vector<VertexBuffer*> m_vertexBuffers;
	IndexBuffer* m_indexBuffer;
};



} // namespace gxeng
} // namespace inl
