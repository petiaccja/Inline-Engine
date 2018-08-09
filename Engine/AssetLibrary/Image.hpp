#pragma once

#define FREEIMAGE_COLORORDER 0
#include <FreeImage/FreeImagePlus.h>

#include <type_traits>
#include <cstdint>
#include <string>


namespace inl {
namespace asset {


enum class eChannelType {
	INT8,
	INT16,
	INT32,
	FLOAT,
};

template <eChannelType channelType>
class ChannelType {
public:
	using type = typename std::conditional<channelType == eChannelType::INT8, uint8_t,
		typename std::conditional<channelType == eChannelType::INT16, uint16_t,
		typename std::conditional<channelType == eChannelType::INT32, uint32_t,
		typename std::conditional<channelType == eChannelType::FLOAT, float, void>::type>::type>::type>::type;
};


template <eChannelType type, size_t count>
class Pixel {
public:
	using channel_type = typename ChannelType<type>::type;

	template <class... Args>
	explicit Pixel(Args... args) {
		constexpr size_t argc = sizeof...(Args);
		static_assert(argc <= count, "More parameters given than channel count.");
		SetArg(0, args...);
		for (size_t i = count; i >= argc; --i) {
			channels[i] = channel_type(0);
		}
	}

	channel_type channels[count];
	channel_type& operator[](int idx) {
		return channels[idx];
	}
	channel_type operator[](int idx) const {
		return channels[idx];
	}
private:
	template <class Head, class... Rest>
	void SetArg(int n, Head head, Rest... rest) {
		channels[n] = head;
		SetArg(n + 1, rest...);
	}
	void SetArg() {}
};


class Image {
public:
	Image() = default;
	Image(const std::string& file);
	~Image() = default;

	size_t GetWidth() const;
	size_t GetHeight() const;
	size_t GetBytesPerRow() const;

	eChannelType GetType() const;
	int GetChannelCount() const;

	void* GetData();
	const void* GetData() const;

	template <eChannelType type, size_t count>
	Pixel<type, count>& At(size_t x, size_t y);
	template <eChannelType type, size_t count>
	const Pixel<type, count>& At(size_t x, size_t y);

	void Create(size_t width, size_t height, eChannelType type, int channelCount);
	void Load(const std::string& file);
private:
	void TranslateImageType(eChannelType& typeOut, size_t& countOut) const;
private:
	fipImage m_image;
};


}
}