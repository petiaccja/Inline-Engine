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

template <class T, class... List>
struct is_any_of;

template <class T>
struct is_any_of<T> {
	static constexpr bool value = false;
};

template <class T, class U, class... List>
struct is_any_of<T, U, List...> {
	static constexpr bool value = std::is_same<T, U>::value || is_any_of<T, List...>::value;
};



class Texture2D {
public:
	Texture2D();

	template <class ViewHeadT, class... ViewT, class = std::enable_if_t<is_any_of<std::decay_t<ViewHeadT>, TextureView2D, RenderTargetView2D, DepthStencilView2D, RWTextureView2D>::value, void>>
	Texture2D(ViewHeadT&& view, ViewT&&... views);

	Texture2D(const Texture2D&);
	Texture2D(Texture2D&&);
	Texture2D& operator=(const Texture2D&);
	Texture2D& operator=(Texture2D&&);
	~Texture2D();

	void AddView(TextureView2D srv);
	void AddView(RenderTargetView2D rtv);
	void AddView(DepthStencilView2D dsv);
	void AddView(RWTextureView2D uav);

	bool Readable() const;
	bool WritableRenderTarget() const;
	bool WritableDepthStencil() const;
	bool WritableRW() const;

	uint64_t Width() const;
	uint32_t Height() const;

	// TODO(Artur) Nodes must be able to modify texture state even if reading only
	// eg. set the texture to pixel shader resource
	const TextureView2D& QueryRead() const;
	const RenderTargetView2D& QueryRenderTarget(CopyCommandList& copyMaker, GraphicsContext& graphicsContext);
	const DepthStencilView2D& QueryDepthStencil(CopyCommandList& copyMaker, GraphicsContext& graphicsContext);
	const RWTextureView2D& QueryRW(CopyCommandList& copyMaker, GraphicsContext& graphicsContext);
private:
	void Clean();
	void CopyIfNeeded(CopyCommandList& copyMaker, GraphicsContext& graphicsContext);

	template <class ViewHeadT, class... ViewTailT>
	void AddViews(ViewHeadT&& head, ViewTailT&&... tail);
	void AddViews() {};

	void GetSize(uint64_t& width, uint32_t& height) const;
private:
	TextureView2D m_srv;
	RenderTargetView2D m_rtv;
	DepthStencilView2D m_dsv;
	RWTextureView2D m_uav;
	mutable bool m_beenCopied;
	mutable bool m_beenUsed;
	std::shared_ptr<std::atomic_size_t> m_usageCount;
};


template <class ViewHeadT, class... ViewT, class>
Texture2D::Texture2D(ViewHeadT&& view, ViewT&&... views) :
	m_beenCopied(false),
	m_beenUsed(false),
	m_usageCount(std::make_shared<std::atomic_size_t>(1))
{
	AddViews(std::forward<ViewHeadT>(view), std::forward<ViewT>(views)...);
}


template <class ViewHeadT, class... ViewTailT>
void Texture2D::AddViews(ViewHeadT&& head, ViewTailT&&... tail) {
	AddView(std::forward<ViewHeadT>(head));
	AddViews(std::forward<ViewTailT>(tail)...);
}


}