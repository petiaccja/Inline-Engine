#pragma once

#include "ResourceView.hpp"

#include <memory>
#include <atomic>


namespace inl::gxeng {
class CopyCommandList;
class GraphicsContext;
}


namespace inl::gxeng::pipeline {


class Texture2D {
public:
	Texture2D();
	Texture2D(TextureView2D srv);
	Texture2D(RenderTargetView2D rtv);
	Texture2D(TextureView2D srv, RenderTargetView2D rtv);

	Texture2D(const Texture2D&);
	Texture2D(Texture2D&&);
	Texture2D& operator=(const Texture2D&);
	Texture2D& operator=(Texture2D&&);
	~Texture2D();

	bool Readable() const;
	bool Writable() const;

	const TextureView2D& QueryRead() const;
	const RenderTargetView2D& QueryWrite(CopyCommandList& copyMaker, GraphicsContext& graphicsContext);
private:
	void Clean();
private:
	TextureView2D m_srv;
	RenderTargetView2D m_rtv;
	mutable bool m_beenCopied;
	mutable bool m_beenUsed;
	std::shared_ptr<std::atomic_size_t> m_usageCount;
};



}