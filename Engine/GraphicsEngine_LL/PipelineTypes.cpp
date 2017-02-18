#include "PipelineTypes.hpp"
#include "GraphicsContext.hpp"


namespace inl::gxeng::pipeline {


Texture2D::Texture2D() {}


Texture2D::Texture2D(const Texture2D& rhs)
	: m_srv(rhs.m_srv),
	m_rtv(rhs.m_rtv),
	m_dsv(rhs.m_dsv),
	m_uav(rhs.m_uav),
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
	m_rtv(std::move(rhs.m_rtv)),
	m_dsv(std::move(rhs.m_dsv)),
	m_uav(std::move(rhs.m_uav)),
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
	m_rtv = rhs.m_rtv;
	m_dsv = rhs.m_dsv;
	m_uav = rhs.m_uav;
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
	m_rtv = std::move(rhs.m_rtv);
	m_dsv = std::move(rhs.m_dsv);
	m_uav = std::move(rhs.m_uav);
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


void Texture2D::AddView(TextureView2D srv) {
	m_srv = std::move(srv);
}
void Texture2D::AddView(RenderTargetView2D rtv) {
	m_rtv = std::move(rtv);
}
void Texture2D::AddView(DepthStencilView2D dsv) {
	m_dsv = std::move(dsv);
}
void Texture2D::AddView(RWTextureView2D uav) {
	m_uav = std::move(uav);
}


bool Texture2D::Readable() const {
	return (bool)m_srv;
}

bool Texture2D::WritableRenderTarget() const {
	return (bool)m_rtv;
}
bool Texture2D::WritableDepthStencil() const {
	return (bool)m_dsv;
}
bool Texture2D::WritableRW() const {
	return (bool)m_uav;
}


const TextureView2D& Texture2D::QueryRead() const {
	m_beenUsed = true;
	return m_srv;
}


const RenderTargetView2D& Texture2D::QueryRenderTarget(CopyCommandList& copyMaker, GraphicsContext& graphicsContext) {
	CopyIfNeeded(copyMaker, graphicsContext);

	return m_rtv;
}

const DepthStencilView2D& Texture2D::QueryDepthStencil(CopyCommandList& copyMaker, GraphicsContext& graphicsContext) {
	CopyIfNeeded(copyMaker, graphicsContext);

	return m_dsv;
}

const RWTextureView2D& Texture2D::QueryRW(CopyCommandList& copyMaker, GraphicsContext& graphicsContext) {
	CopyIfNeeded(copyMaker, graphicsContext);

	return m_uav;
}


void Texture2D::CopyIfNeeded(CopyCommandList& copyMaker, GraphicsContext& graphicsContext) {
	if (!m_beenCopied) {
		m_beenUsed = true;
		m_beenCopied = true;

		// determine if copy is needed
		bool needCopy = *m_usageCount > 1;

		// copy the whole texture
		if (needCopy) {
			//throw std::logic_error("Channeling a single texture to multiple nodes for writing is not supported yet.");
		}
	}
}


}
