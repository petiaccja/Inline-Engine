#pragma once

#include <vector>

#include "Util.hpp"

namespace inl::net
{
	class NetworkBuffer
	{
	public:
		NetworkBuffer()
		{

		}

		~NetworkBuffer()
		{
			util::Delete(Body);
		}

		int BodySize; // size must always be sizeof(int32)
		char *Body;

		bool Valid;
	};
}
