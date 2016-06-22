#pragma once

#include "GraphicsNode.hpp"
#include "Pipeline.hpp"
#include "FrameContext.hpp"


namespace inl {
namespace gxeng {


class Scheduler {
public:
	// don't let anyone else 'own' the pipeline
	void SetPipeline(Pipeline&& pipeline);
	const Pipeline& GetPipeline() const;

	void Execute(FrameContext context);
private:
	Pipeline m_pipeline;
};


}
}
