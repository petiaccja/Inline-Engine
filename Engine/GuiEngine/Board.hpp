#pragma once

#include "Control.hpp"


namespace inl::gui {


class Board {
public:

	void SetDrawingContext(DrawingContext context);
	const DrawingContext& GetDrawingContext() const;
private:
	DrawingContext m_context;


};



}
