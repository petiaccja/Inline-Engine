#pragma once

#include <GraphicsApi_LL/Common.hpp>
#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/Camera2D.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>
#include <GraphicsEngine_LL/OverlayEntity.hpp>
#include <GraphicsEngine_LL/ResourceView.hpp>
#include <GraphicsEngine_LL/Scene.hpp>
#include <GraphicsEngine_LL/TextEntity.hpp>

#include <optional>


namespace inl::gxeng::nodes {



/// <summary>
/// Render 2D TextEntities of the scene.
/// </summary>
/// <remarks>
/// Inputs: Target texture, entities.
/// Outputs: Finished texture.
/// </remarks>
class RenderOverlay : virtual public GraphicsNode,
					  public GraphicsTask,
					  public InputPortConfig<Texture2D, const Camera2D*, const EntityCollection<IOverlayEntity>*, const EntityCollection<ITextEntity>*>,
					  public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "RenderOverlay"; }

	void Initialize(EngineContext& context) override {
		SetTaskSingle(this);
	}

	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

	// Methods not used.
	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;

private:
	void ValidateInput();
	void CreateRtv(SetupContext& context);
	void CreateBinders(SetupContext& context);
	void CreatePipelineStates(SetupContext& context);
	void RenderEntities(GraphicsCommandList& commandList,
						const std::vector<const OverlayEntity*>& overlayList,
						const std::vector<const TextEntity*>& textList,
						float minZ,
						float maxZ);

	// Return the position of the first letter in entity's local space, entity size included.
	static RectF AlignFirstLetter(const TextEntity*);

	// Return weather to draw the entity and an optional clip rect matrix.
	static std::tuple<bool, std::optional<Mat33>> CullEntity(const OverlayEntity& entity, const Mat33& viewProj);
	static std::tuple<bool, std::optional<Mat33>> CullEntity(const TextEntity& entity, const Mat33& viewProj);


private:
	Binder m_overlayBinder;
	Binder m_textBinder;
	BindParameter m_bindOverlayCb;
	BindParameter m_bindOverlayTexture;
	BindParameter m_bindTextTransform;
	BindParameter m_bindTextRender;
	BindParameter m_bindTextTexture;
	std::unique_ptr<gxapi::IPipelineState> m_overlayPso;
	std::unique_ptr<gxapi::IPipelineState> m_textPso;

	RenderTargetView2D m_rtv;
	gxapi::eFormat m_currentFormat = gxapi::eFormat::UNKNOWN;
};


} // namespace inl::gxeng::nodes
