#pragma once

#include <cstdint>

enum class InternalTags : uint32_t
{
	Disconnect = 0xFFFFFFFF,
	Connect = 0xFFFFFFFE,
	AssignID = 0xFFFFFFFD
};