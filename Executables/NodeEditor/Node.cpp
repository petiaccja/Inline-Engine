#include "Node.hpp"
#include <GraphicsEngine/Scene/IScene.hpp>
#include <BaseLibrary/Range.hpp>
#include <regex>
#include <BaseLibrary/StringUtil.hpp>
#undef GetClassName


namespace inl::tool {


static std::string TidyName(std::string arg) {
	using namespace std::regex_constants;
	static std::regex filterClass{ R"(\s*class\s*)", ECMAScript | optimize };
	static std::regex filterPtr64{ R"(\s*__ptr64\s*)", ECMAScript | optimize };
	static std::regex filterConst{ R"(\s*const\s*)", ECMAScript | optimize };
	static std::regex filterInl{ R"(\s*inl::\s*)", ECMAScript | optimize };
	static std::regex filterGxeng{ R"(\s*gxeng::\s*)", ECMAScript | optimize };
	static std::regex filterGxapi{ R"(\s*gxapi::\s*)", ECMAScript | optimize };

	if (arg.find("std::basic_string") == 0) {
		return "std::string";
	}

	arg = std::regex_replace(arg, filterClass, "");
	arg = std::regex_replace(arg, filterPtr64, "");
	arg = std::regex_replace(arg, filterConst, "");

	arg = std::regex_replace(arg, filterInl, "");
	arg = std::regex_replace(arg, filterGxeng, "");
	arg = std::regex_replace(arg, filterGxapi, "");
	return arg;
}


//------------------------------------------------------------------------------
// Link
//------------------------------------------------------------------------------

Link::Link(gxeng::IGraphicsEngine* graphicsEngine, gxeng::IScene* scene) {
	m_scene = scene;
	m_arrowBody.reset(graphicsEngine->CreateOverlayEntity());
	m_arrowBody->SetColor({ 0.8f, 0.8f, 0.8f, 1.f });
	m_arrowBody->SetZDepth(999.f);
}

Link::~Link() {
	auto& entities = m_scene->GetEntities<gxeng::IOverlayEntity>();
	if (entities.Contains(m_arrowBody.get())) {
		entities.Remove(m_arrowBody.get());
	}
}

void Link::SetEndpoints(const Port* source, const Port* target) {
	auto& entities = m_scene->GetEntities<gxeng::IOverlayEntity>();
	if (entities.Contains(m_arrowBody.get())) {
		entities.Remove(m_arrowBody.get());
	}

	if (!source || !target) {
		m_source = m_target = nullptr;
		return;
	}

	m_source = source;
	m_target = target;

	entities.Add(m_arrowBody.get());
	
	UpdatePlacement();
}

void Link::UpdatePlacement() {
	// Also means no source.
	if (!m_target) {
		return;
	}

	// Get source and target points of the arrow.
	Vec2 source = m_source->GetPosition() + Vec2(m_source->GetSize().x/2, 0);
	Vec2 target = m_target->GetPosition() + Vec2(-m_target->GetSize().x/2, 0);

	SetPosition((source + target)/2);
	SetSize(Vec2(Distance(source, target), 2));

	Vec2 dir = (target - source).SafeNormalized();
	m_arrowBody->SetRotation(atan2(dir.y, dir.x));
}

void Link::SetPosition(Vec2 position) {
	m_arrowBody->SetPosition(position);
}

Vec2 Link::GetPosition() const {
	return m_arrowBody->GetPosition();
}

void Link::SetSize(Vec2 size) {
	m_arrowBody->SetScale(size);
}

Vec2 Link::GetSize() const {
	return m_arrowBody->GetScale();
}

void Link::SetDepth(float depth) {
	m_arrowBody->SetZDepth(depth);
}

float Link::GetDepth() const {
	return m_arrowBody->GetZDepth();
}

const Link* Link::Intersect(Vec2 point) const {
	Mat33 invTr = m_arrowBody->GetTransform().Inverse();
	Vec2 invPoint = point * invTr;
	RectF rc{ Vec2{-0.5f, -1.f}, Vec2{0.5f, 1.f} };
	return rc.IsPointInside(invPoint) ? this : nullptr;
}


//------------------------------------------------------------------------------
// Port
//------------------------------------------------------------------------------


Port::Port(gxeng::IGraphicsEngine* graphicsEngine, gxeng::IFont* font, bool isInput, Node& parent, int idx) {
	m_nameLabel.reset(graphicsEngine->CreateTextEntity());
	m_background.reset(graphicsEngine->CreateOverlayEntity());
	m_nameLabel->SetFont(font);
	m_nameLabel->SetFontSize(12.0f);

	m_nameLabel->SetColor(textColor);
	m_background->SetColor(bgColor);

	m_isInput = isInput;

	m_parent = &parent;
	m_index = idx;
}

Port::~Port() {	
	UnlinkAll();
}

void Port::SetName(std::string name) {
	m_nameLabel->SetText(EncodeString<char32_t>(name));
}


void Port::SetPosition(Vec2 position) {
	m_nameLabel->SetPosition(position);
	m_background->SetPosition(position);
}
Vec2 Port::GetPosition() const {
	return m_background->GetPosition();
}


void Port::SetSize(Vec2 size) {
	m_nameLabel->SetSize(size);
	m_background->SetScale(size);
}
Vec2 Port::GetSize() const {
	return m_background->GetScale();
}


void Port::SetDepth(float depth) {
	m_nameLabel->SetZDepth(depth + 0.1f);
	m_background->SetZDepth(depth);
}
float Port::GetDepth() const {
	return m_background->GetZDepth();
}

const gxeng::ITextEntity* Port::GetLabel() const {
	return m_nameLabel.get();
}
const gxeng::IOverlayEntity* Port::GetBackground() const {
	return m_background.get();
}

const Drawable* Port::Intersect(Vec2 point) const {
	Vec2 size = GetSize();
	Vec2 pos = GetPosition();
	RectF rect{ pos - size/2, pos + size/2 };
	bool isInside = rect.IsPointInside(point);
	return isInside ? this : nullptr;
}

Node* Port::GetParent() const {
	return m_parent;
}
int Port::GetIndex() const {
	return m_index;
}
bool Port::IsInput() const {
	return m_isInput;
}

void Port::SetParent(Node* parent) {
	m_parent = parent;
}

void Port::UpdateLinks() {
	for (auto link : m_links) {
		link.second->UpdatePlacement();
	}
}

void Port::Link(Port* other, tool::Link&& link) {
	if (m_links.count(other) != 0) {
		return;
	}
	Port* source = m_isInput ? other : this;
	Port* target = m_isInput ? this : other;
	link.SetEndpoints(source, target);
	auto linkPtr = std::make_shared<tool::Link>(std::move(link));
	LinkUnchecked(other, linkPtr);
	other->LinkUnchecked(this, linkPtr);
}

void Port::Unlink(Port* other) {
	auto it = m_links.find(other);
	if (it != m_links.end()) {
		m_links.erase(it);
	}
	other->UnlinkUnchecked(this);
}

void Port::UnlinkAll() {
	for (auto link : m_links) {
		link.first->UnlinkUnchecked(this);
	}
	m_links.clear();
}

void Port::LinkUnchecked(Port* other, std::shared_ptr<tool::Link> link) {
	m_links.insert_or_assign(other, std::move(link));
}

void Port::UnlinkUnchecked(Port* other) {
	auto it = m_links.find(other);
	if (it != m_links.end()) {
		m_links.erase(it);
	}
}


//------------------------------------------------------------------------------
// Node
//------------------------------------------------------------------------------

Node::Node(Node&& rhs) noexcept {
	m_scene = rhs.m_scene;

	m_myNode = rhs.m_myNode;

	m_nameLabel = std::move(rhs.m_nameLabel);
	m_background = std::move(rhs.m_background);

	m_inputPorts = std::move(rhs.m_inputPorts);
	m_outputPorts = std::move(rhs.m_outputPorts);

	rhs.m_scene = nullptr;
	rhs.m_myNode = nullptr;

	for (auto& p : m_inputPorts) {
		p->SetParent(this);
	}
	for (auto& p : m_outputPorts) {
		p->SetParent(this);
	}
}

Node& Node::operator=(Node&& rhs) noexcept {
	Tidy();

	m_scene = rhs.m_scene;

	m_myNode = rhs.m_myNode;

	m_nameLabel = std::move(rhs.m_nameLabel);
	m_background = std::move(rhs.m_background);

	m_inputPorts = std::move(rhs.m_inputPorts);
	m_outputPorts = std::move(rhs.m_outputPorts);

	rhs.m_scene = nullptr;
	rhs.m_myNode = nullptr;

	for (auto& p : m_inputPorts) {
		p->SetParent(this);
	}
	for (auto& p : m_outputPorts) {
		p->SetParent(this);
	}

	return *this;
}

Node::~Node() {
	Tidy();
}

void Node::SetNode(IGraphEditorNode* node, gxeng::IGraphicsEngine* graphicsEngine, gxeng::IScene* scene, gxeng::IFont* font) {
	Tidy();

	// Create new node.
	std::unique_ptr<gxeng::ITextEntity> nameLabel(graphicsEngine->CreateTextEntity());
	std::unique_ptr<gxeng::IOverlayEntity> background(graphicsEngine->CreateOverlayEntity());
	nameLabel->SetFont(font);
	nameLabel->SetFontSize(12.0f);
	nameLabel->SetColor(textColor);
	nameLabel->SetText(EncodeString<char32_t>(node->GetName() + " : " + TidyName(node->GetClassName())));
	background->SetColor(bgColor);


	// Create new ports.
	std::vector<std::unique_ptr<Port>> inputPorts;
	std::vector<std::unique_ptr<Port>> outputPorts;

	for (auto inputIdx : Range(node->GetNumInputs())) {
		auto p = std::make_unique<Port>(graphicsEngine, font, true, *this, inputIdx);
		p->SetName(node->GetInputName(inputIdx) + " : " + TidyName(node->GetInputTypeName(inputIdx)));
		inputPorts.push_back(std::move(p));
	}
	for (auto outputIdx : Range(node->GetNumOutputs())) {
		auto p = std::make_unique<Port>(graphicsEngine, font, false, *this, outputIdx);
		p->SetName(node->GetOutputName(outputIdx) + " : " + TidyName(node->GetOutputTypeName(outputIdx)));
		outputPorts.push_back(std::move(p));
	}

	try {
		for (auto& p : inputPorts) {
			scene->GetEntities<gxeng::ITextEntity>().Add(p->GetLabel());
			scene->GetEntities<gxeng::IOverlayEntity>().Add(p->GetBackground());
		}
		for (auto& p : outputPorts) {
			scene->GetEntities<gxeng::ITextEntity>().Add(p->GetLabel());
			scene->GetEntities<gxeng::IOverlayEntity>().Add(p->GetBackground());
		}
		scene->GetEntities<gxeng::ITextEntity>().Add(nameLabel.get());
		scene->GetEntities<gxeng::IOverlayEntity>().Add(background.get());
	}
	catch (...) {
		// std::bad_alloc failed inside Scene
		// We can't even remove entities already added, because that may fail with bad_alloc, too.
		std::terminate();
	}

	m_inputPorts = std::move(inputPorts);
	m_outputPorts = std::move(outputPorts);
	m_nameLabel = std::move(nameLabel);
	m_background = std::move(background);
	m_scene = scene;
	m_myNode = node;
}

void Node::SetPosition(Vec2 position) {
	m_nameLabel->SetPosition(position + Vec2(0, -GetSize().y/2 + rowSize/2));
	m_background->SetPosition(position);

	RecalcPortTransforms();
}
Vec2 Node::GetPosition() const {
	return m_background->GetPosition();
}


void Node::SetSize(Vec2 size) {
	if (size.y == 0) {
		size_t numRows = std::max(m_inputPorts.size(), m_outputPorts.size());
		size.y = ((float)numRows - 0.5f)*rowSize + startRow;
	}

	m_background->SetScale(size);
	m_nameLabel->SetSize(Vec2(size.x, rowSize));
	m_nameLabel->SetPosition(GetPosition() + Vec2(0, -GetSize().y/2 + rowSize/2));

	RecalcPortTransforms();
}
Vec2 Node::GetSize() const {
	return m_background->GetScale();
}


void Node::SetDepth(float depth) {
	m_nameLabel->SetZDepth(depth + 0.1f);
	m_background->SetZDepth(depth);

	RecalcPortTransforms();
}
float Node::GetDepth() const {
	return m_background->GetZDepth();
}


IGraphEditorNode* Node::GetNode() const {
	return m_myNode;
}

const Drawable* Node::Intersect(Vec2 point) const {
	Vec2 size = GetSize();
	Vec2 pos = GetPosition();
	RectF rect{ pos - size/2, pos + size/2 };
	bool isInside = rect.IsPointInside(point);

	// Try all ports as well.
	if (isInside) {
		for (auto& p : m_inputPorts) {
			const Drawable* intersect = p->Intersect(point);
			if (intersect) {
				return intersect;
			}
		}
		for (auto& p : m_outputPorts) {
			const Drawable* intersect = p->Intersect(point);
			if (intersect) {
				return intersect;
			}
		}
		return this;
	}
	return nullptr;
}

void Node::UpdateLinks() {
	for (auto& p : m_inputPorts) {
		p->UpdateLinks();
	}
	for (auto& p : m_outputPorts) {
		p->UpdateLinks();
	}
}


void Node::RecalcPortTransforms() {
	float depth = GetDepth();
	Vec2 center = GetPosition();
	Vec2 size = GetSize();
	Vec2 topleft = center - size/2;


	float inputLaneX = center.x - size.x/4;
	float outputLaneX = center.x + size.x/4;

	Vec2 portSize = { size.x/2*0.9, rowSize-2 };

	for (auto i : Range(m_inputPorts.size())) {
		Vec2 pos = { inputLaneX, topleft.y + startRow + i*rowSize };
		m_inputPorts[i]->SetPosition(pos);
		m_inputPorts[i]->SetSize(portSize);
		m_inputPorts[i]->SetDepth(depth + 0.1f);
	}
	for (auto i : Range(m_outputPorts.size())) {
		Vec2 pos = { outputLaneX, topleft.y + startRow + i*rowSize };
		m_outputPorts[i]->SetPosition(pos);
		m_outputPorts[i]->SetSize(portSize);
		m_outputPorts[i]->SetDepth(depth + 0.1f);
	}
}


void Node::Tidy() {
	if (m_scene) {
		auto& texts = m_scene->GetEntities<gxeng::ITextEntity>();
		auto& overlays = m_scene->GetEntities<gxeng::IOverlayEntity>();

		// Clean up previous node.
		texts.Remove(m_nameLabel.get());
		overlays.Remove(m_background.get());

		// Clean up previous ports.
		for (auto& port : m_inputPorts) {
			assert(texts.Contains(port->GetLabel()));

			texts.Remove(port->GetLabel());
			overlays.Remove(port->GetBackground());
		}
		for (auto& port : m_outputPorts) {
			assert(texts.Contains(port->GetLabel()));

			texts.Remove(port->GetLabel());
			overlays.Remove(port->GetBackground());
		}
	}
}



} // namespace inl::tool