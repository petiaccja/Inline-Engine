#pragma once

#include <BaseLibrary/Transformable.hpp>

// Description:
// The link has a source transform, a relative transform and the resulting transform.
// The source transform is another entity, the relative transform is inside this component
// and the resulting transform is the transform of this entity.
//
// Problem #1:
// Need a reference to the source entity's transform.
// If said transform is moved or deleted, we have dangling pointers in all links.
//
// Problem #2:
// For deeper linkage trees, the tree must be updated downwards from the root.
// A simple iteration over components is not able to cope with this.
//
//
// Solution to #1:
// Add events to entities, such as OnDestroy, which could notify derived transforms.
//
// Solution to #2:
// Somehow do a topological sort, which is still linear time. That could be done only if dirty
// and otherwise kept for the next frame.
//
//
// ...Or just fuck the whole thing and do a totally separate system which does not use components at all.
// Starting to make more sense than abusing components to include relations between entities when they
// were absolutely not made for that.



namespace inl::gamelib {


struct RelativeTransformComponent : public Transformable3DN {
	const game::Entity* sourceTransform = nullptr;
};


} // namespace inl::gamelib