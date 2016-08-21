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

			ExecutionResult result;
			result.AddCommandList(std::move(cmdList));
			return result;
		}
	});
	return task;
}



} // namespace gxeng
} // namespace inl
