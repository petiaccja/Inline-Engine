#pragma once

#include <GameLogic/Entity.hpp>
#include <GameLogic/System.hpp>

#ifdef _MSC_VER // disable lemon warnings
#pragma warning(push)
#pragma warning(disable : 4267)
#endif

#include <lemon/list_graph.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace inl::gamelib {


class LinkTransformSystem : public game::SpecificSystem<LinkTransformSystem> {
public:
	LinkTransformSystem();
	LinkTransformSystem(const LinkTransformSystem&);
	LinkTransformSystem& operator=(const LinkTransformSystem&);

	void Update(float elapsed) override;
	void Link(const game::Entity& source, game::Entity& derived);
	void Unlink(game::Entity& derived);

private:
	void TopologicalSort();
	void UpdateTransforms() const;

private:
	lemon::ListDigraph m_graph;
	lemon::ListDigraph::NodeMap<game::Entity*> m_entityMap;
	std::unordered_map<const game::Entity*, lemon::ListDigraph::Node> m_nodeMap;

	bool m_dirty = true;
	std::vector<game::Entity*> m_sortedEntities;
};



} // namespace inl::gamelib