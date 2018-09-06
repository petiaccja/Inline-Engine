#pragma once


#include <type_traits>
#include <cstdint>


namespace inl {
namespace gxeng {


enum class ePixelChannelType {
	INT8_NORM,
	INT16_NORM,
	INT32,
	//FLOAT16,
	FLOAT32,
};

enum class ePixelClass {
	LINEAR,
	VALUE_EXPONENT,
};


namespace impl {

template <ePixelChannelType ChannelType>
class GetChannelType {
public:
	using type = typename std::conditional<ChannelType == ePixelChannelType::INT8_NORM, uint8_t,
		typename std::conditional<ChannelType == ePixelChannelType::INT16_NORM, uint16_t,
		typename std::conditional<ChannelType == ePixelChannelType::INT32, uint32_t,
		//typename std::conditional<ChannelType == ePixelChannelType::FLOAT16, float,
		typename std::conditional<ChannelType == ePixelChannelType::FLOAT32, float, void>::type>::type>::type>::type/*>::type*/;
};


template <class InputT, std::enable_if_t<std::is_floating_point<InputT>::value, int> = 0>
float NormalizeColor(InputT input) {
	return input;
}
template <class InputT, std::enable_if_t<std::is_unsigned<InputT>::value, int> = 0>
float NormalizeColor(InputT input) {
	return float(input) / float(InputT(-1));
}

template <class OutputT, std::enable_if_t<std::is_unsigned<OutputT>::value, int> = 0>
OutputT DenormalizeColor(float input) {
	return OutputT(input * float(~OutputT(0)));
}
template <class OutputT, std::enable_if_t<std::is_floating_point<OutputT>::value, int> = 0>
OutputT DenormalizeColor(float input) {
	return input;
}


template <class T, int Count>
class PixelData;

template <class T>
class PixelData<T, 1> {
public:
	using Type = T;
	PixelData() = default;
	PixelData(T r) : channels{ r } {}
	union {
		T channels[1];
		T red;
		T value;
	};
};

template <class T>
class PixelData<T, 2> {
public:
	using Type = T;
	PixelData() = default;
	PixelData(T r, T g) : channels{ r, g } {}
	union {
		T channels[2];
		struct { T red, green; };
	};
};

template <class T>
class PixelData<T, 3> {
public:
	using Type = T;
	PixelData() = default;
	PixelData(T r, T g, T b) : channels{ r, g, b } {}
	union {
		T channels[3];
		struct { T red, green, blue; };
	};
};

template <class T>
class PixelData<T, 4> {
public:
	using Type = T;
	PixelData() = default;
	PixelData(T r, T g, T b, T a) : channels{ r, g, b, a } {}
	union {
		T channels[4];
		struct { T red, green, blue, alpha; };
	};
};

} // namespace impl


class IPixelReader {
public:
	virtual float Get(const void* pixel, int channel) const = 0;
	virtual void Set(void* pixel, int channel, float value) const = 0;
	virtual ePixelChannelType GetChannelType() const = 0;
	virtual int GetChannelCount() const = 0;
	virtual ePixelClass GetPixelClass() const = 0;
	virtual size_t StructureSize() const = 0;
};

template <ePixelChannelType ChannelType, int ChannelCount, ePixelClass Type>
class Pixel;

template <ePixelChannelType ChannelType, int ChannelCount>
class Pixel<ChannelType, ChannelCount, ePixelClass::LINEAR> 
	: public impl::PixelData<typename impl::GetChannelType<ChannelType>::type, ChannelCount>
{
public:
	using impl::PixelData<typename impl::GetChannelType<ChannelType>::type, ChannelCount>::PixelData;

	class PixelReader : public IPixelReader {
	public:
		float Get(const void* pixel, int channel) const override {
			return impl::NormalizeColor(reinterpret_cast<const Pixel*>(pixel)->channels[channel]);
		}
		void Set(void* pixel, int channel, float value) const override {
			Pixel* px = reinterpret_cast<Pixel*>(pixel);
			px->channels[channel] = impl::DenormalizeColor<typename impl::PixelData<typename impl::GetChannelType<ChannelType>::type, ChannelCount>::Type>(value);
		}
		ePixelChannelType GetChannelType() const override {
			return ChannelType;
		}
		int GetChannelCount() const override {
			return ChannelCount;
		}
		ePixelClass GetPixelClass() const override {
			return ePixelClass::LINEAR;
		}
		size_t StructureSize() const override {
			return sizeof(Pixel);
		}
	};
	static IPixelReader& Reader() {
		return reader;
	}
private:
	static PixelReader reader;
};

template <ePixelChannelType ChannelType, int ChannelCount>
typename Pixel<ChannelType, ChannelCount, ePixelClass::LINEAR>::PixelReader Pixel<ChannelType, ChannelCount, ePixelClass::LINEAR>::reader;


} // namespace gxeng
} // namespace inl