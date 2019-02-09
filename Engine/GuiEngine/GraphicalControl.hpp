#pragma once


#include "GraphicsContext.hpp"


namespace inl::gui {


class GraphicalControl {
public:
	virtual ~GraphicalControl() = default;

	/// <summary> To turn the drawable into a true graphics engine object on a scene, a context can be provided. </summary>
	/// <param name="context"> The graphics engine to use for the entity and the scene to register the entity for. </param>
	virtual void SetContext(const GraphicsContext& context) = 0;

	/// <summary> If no graphics context is available, the drawable will not be visible, but it will be a valid object. </summary>
	virtual void ClearContext() = 0;
};



} // namespace inl::gui