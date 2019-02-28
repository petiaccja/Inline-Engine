#include <BaseLibrary/Serialization/BinarySerializer.hpp>

#include <Catch2/catch.hpp>
#include <cstdint>

using namespace inl;


struct SerializerTestData {
	char a;
	int b;
	float c;
};


BinarySerializer& operator<<(BinarySerializer& stream, const SerializerTestData& obj) {
	stream << obj.a;
	stream << obj.b;
	stream << obj.c;
	return stream;
}


BinarySerializer& operator>>(BinarySerializer& stream, SerializerTestData& obj) {
	stream >> obj.a;
	stream >> obj.b;
	stream >> obj.c;
	return stream;
}


TEST_CASE("BinarySerializer - Integers", "[BinarySerializer]") {
	BinarySerializer serializer;

	int8_t i8 = -123;
	uint8_t u8 = 123;
	int16_t i16 = -12345;
	uint16_t u16 = 12345;
	int32_t i32 = -12345678;
	uint32_t u32 = 12345678;
	int64_t i64 = -1234567812345678;
	uint64_t u64 = 1234567812345678;

	serializer << i8 << u8 << i16 << u16 << i32 << u32 << i64 << u64;

	int8_t i8_rep;
	uint8_t u8_rep;
	int16_t i16_rep;
	uint16_t u16_rep;
	int32_t i32_rep;
	uint32_t u32_rep;
	int64_t i64_rep;
	uint64_t u64_rep;

	serializer >> i8_rep >> u8_rep >> i16_rep >> u16_rep >> i32_rep >> u32_rep >> i64_rep >> u64_rep;

	REQUIRE(i8 == i8_rep);
	REQUIRE(u8 == u8_rep);
	REQUIRE(i16 == i16_rep);
	REQUIRE(u16 == u16_rep);
	REQUIRE(i32 == i32_rep);
	REQUIRE(u32 == u32_rep);
	REQUIRE(i64 == i64_rep);
	REQUIRE(u64 == u64_rep);
}


TEST_CASE("BinarySerializer - Floats", "[BinarySerializer]") {
	BinarySerializer serializer;

	int8_t f32 = -123.1415f;
	uint8_t f64 = 123.141592654;

	serializer << f32 << f64;

	int8_t f32_rep;
	uint8_t f64_rep;

	serializer >> f32_rep >> f64_rep;

	REQUIRE(f32 == f32_rep);
	REQUIRE(f64 == f64_rep);
}


TEST_CASE("BinarySerializer - Struct", "[BinarySerializer]") {
	BinarySerializer serializer;

	SerializerTestData data{ 'a', 96000, 88.44f };
	SerializerTestData data_rep;

	serializer << data;
	serializer >> data_rep;

	REQUIRE(data.a == data_rep.a);
	REQUIRE(data.b == data_rep.b);
	REQUIRE(data.c == data_rep.c);
}


TEST_CASE("BinarySerializer - Underflow error", "[BinarySerializer]") {
	BinarySerializer serializer;

	serializer << (uint8_t)16;
	uint32_t tooMuch;

	REQUIRE_THROWS(serializer >> tooMuch);
}