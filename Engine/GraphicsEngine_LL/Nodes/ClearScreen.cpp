#include "ClearScreen.h"

namespace inl {
namespace gxeng {



Task ClearScreen::GetTask() {
	Task task;
	task.InitSequential({
		[this](const ExecutionContext& context) -> ExecutionResult
		{
			GraphicsCommandList cmdList = context.GetGraphicsCommandList();

			// TODO: clear render target...
			if (m_target) {
				// get color
				gxapi::ColorRGBA clearColor = this->GetInput<0>().Get();

				// set viewport and scissor rects
				gxapi::Viewport viewport;
				viewport.width = m_target->GetWidth();
				viewport.height = m_target->GetHeight();
				viewport.topLeftX = 0;
				viewport.topLeftY = 0;
				viewport.minDepth = 0;
				viewport.maxDepth = 1.0f;
				cmdList.SetViewports(1, &viewport);

				gxapi::Rectangle scissorRect{0, (int)m_target->GetHeight(), 0, (int)m_target->GetWidth()};
				cmdList.SetScissorRects(1, &scissorRect);

				// set an rtv barrier
				// cmdList.ResourceBarrier(gxapi::TransitionBarrier{m_target->})

				// clear rtv
				cmdList.SetRenderTargets(1, &m_target, nullptr);
				cmdList.ClearRenderTarget(m_target, clearColor);
			}

			ExecutionResult result;
			result.AddCommandList(std::move(cmdList));
			return result;
		}
	});
	return task;
}


void ClearScreen::SetTarget(Texture2D* target) {
	m_target = target;
}


} // namespace gxeng
} // namespace inl
