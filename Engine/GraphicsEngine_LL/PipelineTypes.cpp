#include "PipelineTypes.hpp"
#include "GraphicsContext.hpp"


namespace inl::gxeng::pipeline {


Texture2D::Texture2D() {}


Texture2D::Texture2D(TextureView2D srv)
	: Texture2D(srv, RenderTargetView2D())
{}


Texture2D::Texture2D(RenderTargetView2D rtv)
	: Texture2D({}, rtv)
{}


Texture2D::Texture2D(TextureView2D srv, RenderTargetView2D rtv)
	: m_srv(std::move(srv)),
	m_renderTexture({RenderTextureType::RENDER_TARGET, rtv, {}}),
	m_beenCopied(false),
	m_beenUsed(false),
	m_usageCount(std::make_shared<std::atomic_size_t>(1))
{}


Texture2D::Texture2D(TextureView2D srv, DepthStencilView2D dsv)
	: m_srv(std::move(srv)),
	m_renderTexture({ RenderTextureType::DEPTH_STENCIL, {}, dsv }),
	m_beenCopied(false),
	m_beenUsed(false),
	m_usageCount(std::make_shared<std::atomic_size_t>(1))
{}


Texture2D::Texture2D(const Texture2D& rhs) 
	: m_srv(rhs.m_srv),
	m_renderTexture(rhs.m_renderTexture),
	m_beenCopied(false),
	m_beenUsed(false),
	m_usageCount(rhs.m_usageCount)
{
	if (m_usageCount) {
		++*m_usageCount;
	}
}


Texture2D::Texture2D(Texture2D&& rhs)
	: m_srv(std::move(rhs.m_srv)),
	m_renderTexture(std::move(rhs.m_renderTexture)),
	m_beenCopied(rhs.m_beenCopied),
	m_beenUsed(rhs.m_beenUsed),
	m_usageCount(std::move(rhs.m_usageCount)) 
{
	rhs.m_beenUsed = false;
	rhs.m_beenCopied = false;
}


Texture2D& Texture2D::operator=(const Texture2D& rhs) {
	Clean();

	m_srv = rhs.m_srv;
	m_renderTexture = rhs.m_renderTexture;
	m_usageCount = rhs.m_usageCount;
	m_beenUsed = false;
	m_beenCopied = false;
	if (m_usageCount) {
		++*m_usageCount;
	}

	return *this;
}


Texture2D& Texture2D::operator=(Texture2D&& rhs) {
	Clean();

	m_srv = std::move(rhs.m_srv);
	m_renderTexture = std::move(rhs.m_renderTexture);
	m_usageCount = std::move(rhs.m_usageCount);
	m_beenUsed = rhs.m_beenUsed;
	m_beenCopied = rhs.m_beenCopied;

	rhs.m_beenUsed = false;
	rhs.m_beenCopied = false;

	return *this;
}


Texture2D::~Texture2D() {
	Clean();
}


void Texture2D::Clean() {
	if (!m_beenUsed && m_usageCount) {
		--*m_usageCount;
	}
}



bool Texture2D::Readable() const {
	return (bool)m_srv;
}
bool Texture2D::Writable() const {
	switch (m_renderTexture.type) {
	case RenderTextureType::RENDER_TARGET:
		return (bool)m_renderTexture.rtv;
	case RenderTextureType::DEPTH_STENCIL:
		return (bool)m_renderTexture.dsv;
	default:
		assert(false);
	}
	return false;
}


const TextureView2D& Texture2D::QueryRead() const {
	m_beenUsed = true;
	return m_srv;
}


const RenderTexture2D& Texture2D::QueryWrite(CopyCommandList& copyMaker, GraphicsContext& graphicsContext) {
	if (!m_beenCopied) {
		m_beenUsed = true;
		m_beenCopied = true;

		// determine if copy is needed
		bool needCopy = *m_usageCount > 1;
		if (!needCopy) {
			return m_renderTexture;
		}

		// copy the whole texture
		throw std::logic_error("Channeling a single texture to multiple nodes for writing is not supported yet.");
	}
}



}
