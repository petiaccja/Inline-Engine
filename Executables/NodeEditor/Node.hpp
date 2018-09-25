#pragma once

#include "Drawable.hpp"

#include <BaseLibrary/GraphEditor/IGraphEditorNode.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GraphicsEngine/Scene/IOverlayEntity.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>
#include <GraphicsEngine/Scene/ITextEntity.hpp>

#include <InlineMath.hpp>
#include <map>
#include <memory>


namespace inl::tool {


class Port;
class Node;


class Link : public Drawable {
public:
	Link(gxeng::IGraphicsEngine* graphicsEngine, gxeng::IScene* m_scene);
	Link(const Link&) = delete;
	Link(Link&&) = default;
	~Link();

	// Endpoints.
	void SetEndpoints(const Port* source, const Port* target);

	// Positioning.
	void UpdatePlacement();

	void SetPosition(Vec2 position) override;
	Vec2 GetPosition() const override;

	void SetSize(Vec2 size) override;
	Vec2 GetSize() const override;

	void SetDepth(float depth) override;
	float GetDepth() const override;

	const Link* Intersect(Vec2 point) const override;

	const Port* GetTarget() const { return m_target; }
	const Port* GetSource() const { return m_source; }

private:
	std::unique_ptr<gxeng::IOverlayEntity> m_arrowBody;
	const Port* m_source;
	const Port* m_target;
	gxeng::IScene* m_scene;
};


class Port : public Drawable {
public:
	Port(gxeng::IGraphicsEngine* graphicsEngine, gxeng::IFont* font, bool isInput, Node& parent, int idx);
	Port(Port&&) = default;
	Port(const Port&) = delete;
	Port& operator=(Port&&) = default;
	Port& operator=(const Port&) = delete;
	~Port();

	void SetName(std::string name);

	void SetPosition(Vec2 position) override;
	Vec2 GetPosition() const override;

	void SetSize(Vec2 size) override;
	Vec2 GetSize() const override;

	void SetDepth(float depth) override;
	float GetDepth() const override;

	const gxeng::ITextEntity* GetLabel() const;
	const gxeng::IOverlayEntity* GetBackground() const;

	const Drawable* Intersect(Vec2 point) const override;

	Node* GetParent() const;
	int GetIndex() const;
	bool IsInput() const;

	void SetParent(Node* parent);

	void UpdateLinks();

	void Link(Port* other, tool::Link&& link);
	void Unlink(Port* other);
	void UnlinkAll();

	const std::map<Port*, std::shared_ptr<tool::Link>>& GetLinks() const { return m_links; }

private:
	void LinkUnchecked(Port* other, std::shared_ptr<tool::Link> link);
	void UnlinkUnchecked(Port* other);

private:
	bool m_isInput;

	std::map<Port*, std::shared_ptr<tool::Link>> m_links;

	std::unique_ptr<gxeng::ITextEntity> m_nameLabel;
	std::unique_ptr<gxeng::IOverlayEntity> m_background;

	Node* m_parent;
	int m_index;

	inline static const Vec4 textColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	inline static const Vec4 bgColor = { 0.15f, 0.15f, 0.15f, 1.0f };
};



class Node : public Drawable {
public:
	Node() = default;
	Node(const Node&) = delete;
	Node(Node&&) noexcept;
	Node& operator=(const Node&) = delete;
	Node& operator=(Node&&) noexcept;
	~Node();

	void SetNode(IGraphEditorNode* node, gxeng::IGraphicsEngine* graphicsEngine, gxeng::IScene* scene, gxeng::IFont* font);

	void SetPosition(Vec2 position) override;
	Vec2 GetPosition() const override;

	void SetSize(Vec2 size) override;
	Vec2 GetSize() const override;

	void SetDepth(float depth) override;
	float GetDepth() const override;

	IGraphEditorNode* GetNode() const;

	const Drawable* Intersect(Vec2 point) const override;

	void UpdateLinks();

	const std::vector<std::unique_ptr<Port>>& GetInputs() const { return m_inputPorts; }
	const std::vector<std::unique_ptr<Port>>& GetOutputs() const { return m_outputPorts; }

private:
	void RecalcPortTransforms();
	void Tidy();

private:
	gxeng::IScene* m_scene = nullptr;

	IGraphEditorNode* m_myNode = nullptr;

	std::unique_ptr<gxeng::ITextEntity> m_nameLabel;
	std::unique_ptr<gxeng::IOverlayEntity> m_background;

	std::vector<std::unique_ptr<Port>> m_inputPorts;
	std::vector<std::unique_ptr<Port>> m_outputPorts;

	static constexpr float startRow = 45.f;
	static constexpr float rowSize = 30.f;
	inline static const Vec4 textColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	inline static const Vec4 bgColor = { 0.3f, 0.3f, 0.3f, 1.0f };
};


} // namespace inl::tool