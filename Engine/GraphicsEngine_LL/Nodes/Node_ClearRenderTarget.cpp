#include "Node_ClearRenderTarget.h"

namespace inl {
namespace gxeng {
namespace nodes {



Task ClearRenderTarget::GetTask() {
	Task task;
	task.InitSequential({
		[this](const ExecutionContext& context) -> ExecutionResult
		{
			ExecutionResult result;

			// Update ports.
			//std::shared_ptr<BackBuffer> target(this->GetInput<0>().Get(), [](BackBuffer*){});
			auto target = this->GetInput<0>().Get();
			auto targetSurface = target.GetResource().get();

			this->GetInput<0>().Clear();
			this->GetOutput<0>().Set(target);

			//const auto& targetTexture = target->GetResource();

			// Clear render target
			if (targetSurface) {
				// Acquire a command list.
				GraphicsCommandList cmdList = context.GetGraphicsCommandList();

				// get color
				gxapi::ColorRGBA clearColor = this->GetInput<1>().Get();

				// set viewport and scissor rects
				gxapi::Viewport viewport;
				viewport.width = (float)targetSurface->GetWidth();
				viewport.height = (float)targetSurface->GetHeight();
				viewport.topLeftX = 0;
				viewport.topLeftY = 0;
				viewport.minDepth = 0;
				viewport.maxDepth = 1.0f;
				cmdList.SetViewports(1, &viewport);

				gxapi::Rectangle scissorRect{0, (int)targetSurface->GetHeight(), 0, (int)targetSurface->GetWidth()};
				cmdList.SetScissorRects(1, &scissorRect);


				// clear rtv
				auto pRTV = &target;
				cmdList.SetResourceState(target.GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
				cmdList.SetRenderTargets(1, &pRTV, nullptr);
				cmdList.ClearRenderTarget(target, clearColor);

				// Output command list.
				result.AddCommandList(std::move(cmdList));
			}
			
			return result;
		}
	});
	return task;
}


} // namespace nodes
} // namespace gxeng
} // namespace inl
