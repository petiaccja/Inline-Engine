#pragma once

#include "../GraphicsNode.hpp"
#include <BaseLibrary/Logging_All.hpp>

#include <sstream>


namespace inl {
namespace gxeng {



class FrameCounter : 
	public exc::InputPortConfig<>,
	public exc::OutputPortConfig<unsigned long long>
{
public:
	FrameCounter() { m_log = nullptr; }

	virtual void Update() override {
		++m_frameCount;
		GetOutput<0>().Set(m_frameCount);

		std::stringstream message;
		message << "Frame # " << m_frameCount;

		if (m_log) {
			m_log->Event(message.str());
		}
	}

	virtual void Notify(exc::InputPortBase* sender) override {}

	void SetLog(exc::LogStream* log) {
		m_log = log;
	}
private:
	unsigned long long m_frameCount = 0;
	exc::LogStream* m_log;
};



} // namespace gxeng
} // namespace inl
