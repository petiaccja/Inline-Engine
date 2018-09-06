#pragma once

#include <GraphicsEngine/Resources/Vertex.hpp>
#include <memory>


namespace inl::gxeng {


class SemanticCompressor {
public:
	virtual ~SemanticCompressor() {}

	virtual void Compress(const void* input, void* output) const = 0;
	virtual int Size() const = 0;
	virtual bool IsSupported(eVertexElementSemantic semantic) const = 0;
};


class NormalCompressor : public SemanticCompressor {
public:
	void Compress(const void* input, void* output) const override;
	int Size() const override;
	bool IsSupported(eVertexElementSemantic semantic) const override;
};


class ColorCompressor : public SemanticCompressor {
public:
	void Compress(const void* input, void* output) const override;
	int Size() const override;
	bool IsSupported(eVertexElementSemantic semantic) const override;
};


class PassthroughCompressor : public SemanticCompressor {
public:
	void Setup(eVertexElementSemantic semantic, const IVertexReader* reader);

	void Compress(const void* input, void* output) const override;
	int Size() const override;
	bool IsSupported(eVertexElementSemantic semantic) const override;
private:
	int m_stride = 0;
};





class VertexCompressor {
public:
	VertexCompressor(const IVertexReader* reader, const std::vector<bool>& elementMap);

	std::vector<uint8_t> GetCompressedStream(const VertexBase* vertices, size_t vertexCount) const;
	int GetCompressedStride() const;
	std::vector<int> GetCompressedOffsets() const;

private:
	SemanticCompressor* AssignCompressor(const IVertexReader::Element& element);
	void CreateDefaultCompressorList();

private:
	struct CompressionElement {
		IVertexReader::Element sourceElement;
		SemanticCompressor* assignedCompressor;
	};

	const IVertexReader* m_reader;
	std::vector<CompressionElement> m_elementsToCompress;
	std::vector<std::unique_ptr<SemanticCompressor>> m_availableCompressors;
	std::vector<std::unique_ptr<PassthroughCompressor>> m_passThroughCompressors;
};



} // namespace inl::gxeng