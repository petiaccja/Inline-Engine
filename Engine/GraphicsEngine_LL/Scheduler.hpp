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
	Pipeline ReleasePipeline();
	void Execute(FrameContext context);
protected:
	static void MakeResident(std::vector<GenericResource*> usedResources);
	static void Evict(std::vector<GenericResource*> usedResources);

	static void EnqueueCommandList(CommandQueue& commandQueue, 
								   BasicCommandList commandList,
								   const FrameContext& context);
	static void RenderFailureScreen(FrameContext context);
private:
	Pipeline m_pipeline;
};


}
}
