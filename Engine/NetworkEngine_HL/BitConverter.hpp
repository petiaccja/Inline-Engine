#pragma once

// i want to use bitshifting but i red in stackoverflow that casting doesnt generate overhead
// now ive hit a wall
// casting vs bitshifting

#include <cstdint>
#include <cstring>

#include <BaseLibrary/Exception/Exception.hpp>

namespace inl
{
	class BitConverter
	{
	public:
		template<typename T>
		inline static uint8_t *ToBytes(T value)
		{
			uint8_t *data = new uint8_t[sizeof(T)]();
			memcpy(data, &value, sizeof(T));
			return data;
		}

		template<typename T>
		inline static T FromBytes(uint8_t *data)
		{
			if (!data)
				throw InvalidArgumentException("cant have null parameter -> BitConverter::FromBytes");
			T value;
			memcpy(&value, data, sizeof(T));
			return value;
		}
	};
}