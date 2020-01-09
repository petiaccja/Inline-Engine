#pragma once

#include <cstdint>


namespace inl:: gxeng {



class PipelineEventListener {
public:
	virtual void OnFrameBeginDevice(uint64_t frameId) = 0;
	virtual void OnFrameBeginHost(uint64_t frameId) = 0;
	virtual void OnFrameBeginAwait(uint64_t frameId) = 0;
	virtual void OnFrameCompleteDevice(uint64_t frameId) = 0;
	virtual void OnFrameCompleteHost(uint64_t frameId) = 0;
};


} // namespace gxeng
