#pragma once

namespace inl::net
{
	static void Delete(void *ptr)
	{
		delete ptr;
		ptr = nullptr;
	}
}