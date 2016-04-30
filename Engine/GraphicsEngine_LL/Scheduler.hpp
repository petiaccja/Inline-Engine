#pragma once

#include "GraphicsNode.hpp"
#include "Pipeline.hpp"


namespace inl {
namespace gxeng {


class Scheduler {
public:
	// don't let anyone else 'own' the pipeline
	void SetPipeline(Pipeline&& pipeline);
	const Pipeline& GetPipeline() const;

	void Execute();
private:
	Pipeline m_pipeline;
};


}
}
