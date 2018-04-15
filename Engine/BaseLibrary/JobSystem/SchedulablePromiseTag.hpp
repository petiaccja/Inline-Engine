#pragma once

namespace inl::jobs {


class Scheduler;

struct SchedulablePromiseTag {
	Scheduler* m_scheduler = nullptr;
};


}