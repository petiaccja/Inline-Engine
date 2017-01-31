#pragma once

#include "ResourceView.hpp"

#include <memory>
#include <atomic>


namespace inl::gxeng {
class CopyCommandList;
class GraphicsContext;
}


namespace inl::gxeng::pipeline {


enum class RenderTextureType { RENDER_TARGET, DEPTH_STENCIL };

struct RenderTexture2D {
	RenderTextureType type;

	// Could be a union but would need to explicitly define the
	// constructors and destructor for this struct
	// which is too much boilerplate for so little performance gain.
	RenderTargetView2D rtv;
	DepthStencilView2D dsv;
};


class Texture2D {
public:
	Texture2D();
	Texture2D(TextureView2D srv);
	Texture2D(RenderTargetView2D rtv);
	Texture2D(TextureView2D srv, RenderTargetView2D rtv);
	Texture2D(TextureView2D srv, DepthStencilView2D dsv);

	Texture2D(const Texture2D&);
	Texture2D(Texture2D&&);
	Texture2D& operator=(const Texture2D&);
	Texture2D& operator=(Texture2D&&);
	~Texture2D();

	bool Readable() const;
	bool Writable() const;

	// TODO(Artur) Nodes must be able to modify texture state even if reading only
	// eg. set the texture to pixel shader resource
	const TextureView2D& QueryRead() const;
	const RenderTexture2D& QueryWrite(CopyCommandList& copyMaker, GraphicsContext& graphicsContext);
private:
	void Clean();
private:
	TextureView2D m_srv;
	RenderTexture2D m_renderTexture;
	mutable bool m_beenCopied;
	mutable bool m_beenUsed;
	std::shared_ptr<std::atomic_size_t> m_usageCount;
};



}