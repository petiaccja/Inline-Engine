#include "UploadTask.hpp"


namespace inl::gxeng {

void UploadTask::Setup(SetupContext& context) {
	return;
}
void UploadTask::Execute(RenderContext& context) {
	CopyCommandList& commandList = context.AsCopy();

	for (auto& request : *m_uploads) {
		// Init copy parameters
		auto& source = request.source;
		auto& destination = request.destination;

		// Set resource states
		commandList.SetResourceState(destination, gxapi::eResourceState::COPY_DEST);

		auto destType = request.destType;

		if (destType == UploadManager::DestType::BUFFER) {
			auto& dstBuffer = static_cast<const LinearBuffer&>(destination);
			commandList.CopyBuffer(dstBuffer, request.dstOffsetX, source, 0, source.GetSize());
		}
		else if (destType == UploadManager::DestType::TEXTURE_2D) {
			auto& dstTexture = static_cast<const Texture2D&>(destination);
			SubTexture2D dstPlace(request.dstSubresource, Vector<intptr_t, 2>((intptr_t)request.dstOffsetX, (intptr_t)request.dstOffsetY));
			commandList.CopyTexture(dstTexture, source, dstPlace, request.textureBufferDesc);
		}
	}
}


} // namespace inl::gxeng
