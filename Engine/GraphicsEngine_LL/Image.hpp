#pragma once

#include <memory>
#include "GpuBuffer.hpp"


namespace inl {
namespace gxeng {


enum class eImageBitDepth {
	INT8,
	INT16,
	INT32,
	FLOAT16,
	FLOAT32,
};

enum class eImageChannels {
	MONO,
	RGB,
	RGBA,
	RGBE,
};



class Image {
public:
	Image();
	~Image();

	size_t Width();
	size_t Height();
private:
	std::shared_ptr<Texture2DSRV> m_resource;
};


} // namespace gxeng
} // namespace inl
