#include "LinkTransformSystem.hpp"

#include "../Components/RelativeTransformComponent.hpp"
#include "../Components/TransformComponent.hpp"

#ifdef _MSC_VER // disable lemon warnings
#pragma warning(push)
#pragma warning(disable : 4267)
#endif

#include <lemon/connectivity.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace inl::gamelib {

using namespace game;


LinkTransformSystem::LinkTransformSystem()
	: m_entityMap(m_graph) {}


LinkTransformSystem::LinkTransformSystem(const LinkTransformSystem& rhs) : m_entityMap(m_graph) {
	digraphCopy(rhs.m_graph, m_graph).nodeMap(rhs.m_entityMap, m_entityMap);
}


LinkTransformSystem& LinkTransformSystem::operator=(const LinkTransformSystem& rhs) {
	digraphCopy(rhs.m_graph, m_graph).nodeMap(rhs.m_entityMap, m_entityMap);
	return *this;
}


void LinkTransformSystem::Update(float elapsed) {
	if (m_dirty) {
		TopologicalSort();
		m_dirty = false;
	}

	UpdateTransforms();
}


void LinkTransformSystem::Link(const Entity& source, Entity& derived) {
	if (derived.HasComponent<RelativeTransformComponent>()) {
		throw InvalidArgumentException("Derived entity's transform is already linked to another.");
	}

	auto sourceIt = m_nodeMap.find(&source);
	if (sourceIt == m_nodeMap.end()) {
		sourceIt = m_nodeMap.insert({ &source, m_graph.addNode() }).first;
	}

	auto [derivedIt, derivedNewlyInserted] = m_nodeMap.insert({ &derived, m_graph.addNode() });
	assert(derivedNewlyInserted);

	const auto& sourceNode = sourceIt->second;
	const auto& derivedNode = derivedIt->second;

	m_graph.addArc(sourceNode, derivedNode);
}


void LinkTransformSystem::Unlink(Entity& derived) {
	const auto derivedIt = m_nodeMap.find(&derived);
	if (derivedIt == m_nodeMap.end()) {
		throw InvalidArgumentException("Entity's transform is not linked.");
	}

	assert(countInArcs(m_graph, derivedIt->second) == 1);
	lemon::ListDigraph::InArcIt arc(m_graph, derivedIt->second);
	auto sourceNode = m_graph.source(arc);
	bool eraseSource = countOutArcs(m_graph, sourceNode);

	m_graph.erase(derivedIt->second);
	m_nodeMap.erase(derivedIt);
	if (eraseSource) {
		const auto* source = m_entityMap[sourceNode];
		m_graph.erase(sourceNode);
		m_nodeMap.erase(source);
	}

	derived.RemoveComponent<RelativeTransformComponent>();
}


void LinkTransformSystem::TopologicalSort() {
	lemon::ListDigraph::NodeMap<size_t> orderMap(m_graph);
	const bool isDag = checkedTopologicalSort(m_graph, orderMap);
	if (!isDag) {
		throw InvalidStateException("Linkage of entity transforms contains a directed circle.");
	}

	std::vector<Entity*> sortedEntities(countNodes(m_graph));
	for (lemon::ListDigraph::NodeIt it(m_graph); it != lemon::INVALID; ++it) {
		sortedEntities[orderMap[it]] = m_entityMap[it];
	}

	m_sortedEntities = std::move(sortedEntities);
}


Transform3D CombineTransform(const Transform3D& source, const Transform3D& relative) {
	Transform3D combined = relative;

	// In case of PR only, avoid the SVD by ignoring the scale which is roughly (1,1,1).
	if ((source.GetScale() - Vec3(1, 1, 1)).LengthSquared() < 0.00001f) {
		combined.Rotate(source.GetRotation());
		combined.Move(source.GetPosition());
	}
	// Do full transform chaining.
	else {
		combined.Rotate(source.GetShearRotation());
		combined.Scale(source.GetScale()); // This needs to do a 3x3 SVD, which is very expensive.
		combined.Rotate(source.GetPostRotation());
		combined.Move(source.GetPosition());
	}

	return combined;
}


void LinkTransformSystem::UpdateTransforms() const {
	for (auto entity : m_sortedEntities) {
		try {
			auto& absoluteTransform = entity->GetFirstComponent<TransformComponent>();
			const auto& relativeTransform = entity->GetFirstComponent<RelativeTransformComponent>();
			const auto& sourceTransform = relativeTransform.sourceTransform->GetFirstComponent<TransformComponent>();
			static_cast<Transform3D&>(absoluteTransform) = CombineTransform(sourceTransform, relativeTransform);
		}
		catch (...) {
		}
	}
}


} // namespace inl::gamelib