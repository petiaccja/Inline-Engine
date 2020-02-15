#pragma once

#include "Native.hpp"

#include <BaseLibrary/EnumFlag.hpp>

#include <cstdint>
#include <limits>
#include <string>
#undef DOMAIN // math.h, conflicting with eShaderVisibility::DOMAIN
#include "Exception.hpp"

#include "../GraphicsApi_LL/DisableWin32Macros.h"

#include <cassert>
#include <memory>
#include <vector>

namespace inl::gxapi {


class ICommandAllocator;
class IRootSignature;
class IPipelineState;
class IResource;


//------------------------------------------------------------------------------
// Common structures
//------------------------------------------------------------------------------


struct Rectangle {
	Rectangle() = default;
	Rectangle(int top, int bottom, int left, int right)
		: top(top), bottom(bottom), left(left), right(right) {}

	int Width() const { return right - left; }
	int Height() const { return top - bottom; }
	int CenterX() const { return (right + left) / 2; }
	int CenterY() const { return (top + bottom) / 2; }
	int Area() const { return Width() * Height(); }

	int top, bottom, left, right;
};


struct Cube {
	Cube(int top, int bottom, int left, int right, int front, int back)
		: top(top), bottom(bottom), left(left), right(right), front(front), back(back) {}

	int top, bottom, left, right, front, back;
};


struct ColorRGBA {
	ColorRGBA(float r = 0, float g = 0, float b = 0, float a = 1)
		: r(r), g(g), b(b), a(a) {}

	float r, g, b, a;
};


//------------------------------------------------------------------------------
// Enumerations
//------------------------------------------------------------------------------


enum class eShaderType {
	VERTEX,
	PIXEL,
	DOMAIN,
	HULL,
	GEOMETRY,
	COMPUTE,
};


enum class eFormat {
	UNKNOWN = 0,

	R32G32B32A32_TYPELESS = 1,
	R32G32B32A32_FLOAT = 2,
	R32G32B32A32_UINT = 3,
	R32G32B32A32_SINT = 4,

	R32G32B32_TYPELESS = 5,
	R32G32B32_FLOAT = 6,
	R32G32B32_UINT = 7,
	R32G32B32_SINT = 8,

	R16G16B16A16_TYPELESS = 9,
	R16G16B16A16_FLOAT = 10,
	R16G16B16A16_UNORM = 11,
	R16G16B16A16_UINT = 12,
	R16G16B16A16_SNORM = 13,
	R16G16B16A16_SINT = 14,

	R32G32_TYPELESS = 15,
	R32G32_FLOAT = 16,
	R32G32_UINT = 17,
	R32G32_SINT = 18,

	R32G8X24_TYPELESS = 19,
	D32_FLOAT_S8X24_UINT = 20,
	R32_FLOAT_X8X24_TYPELESS = 21,
	X32_TYPELESS_G8X24_UINT = 22,

	R10G10B10A2_TYPELESS = 23,
	R10G10B10A2_UNORM = 24,
	R10G10B10A2_UINT = 25,
	R11G11B10_FLOAT = 26,

	R8G8B8A8_TYPELESS = 27,
	R8G8B8A8_UNORM = 28,
	R8G8B8A8_UNORM_SRGB = 29,
	R8G8B8A8_UINT = 30,
	R8G8B8A8_SNORM = 31,
	R8G8B8A8_SINT = 32,

	R16G16_TYPELESS = 33,
	R16G16_FLOAT = 34,
	R16G16_UNORM = 35,
	R16G16_UINT = 36,
	R16G16_SNORM = 37,
	R16G16_SINT = 38,

	R32_TYPELESS = 39,
	D32_FLOAT = 40,
	R32_FLOAT = 41,
	R32_UINT = 42,
	R32_SINT = 43,

	R24G8_TYPELESS = 44,
	D24_UNORM_S8_UINT = 45,
	R24_UNORM_X8_TYPELESS = 46,
	X24_TYPELESS_G8_UINT = 47,

	R8G8_TYPELESS = 48,
	R8G8_UNORM = 49,
	R8G8_UINT = 50,
	R8G8_SNORM = 51,
	R8G8_SINT = 52,

	R16_TYPELESS = 53,
	R16_FLOAT = 54,
	D16_UNORM = 55,
	R16_UNORM = 56,
	R16_UINT = 57,
	R16_SNORM = 58,
	R16_SINT = 59,

	R8_TYPELESS = 60,
	R8_UNORM = 61,
	R8_UINT = 62,
	R8_SNORM = 63,
	R8_SINT = 64,
	A8_UNORM = 65,

	//R1_UNORM = 66,
	//R9G9B9E5_SHAREDEXP = 67,
	//R8G8_B8G8_UNORM = 68,
	//G8R8_G8B8_UNORM = 69,

	//BC1_TYPELESS = 70,
	//BC1_UNORM = 71,
	//BC1_UNORM_SRGB = 72,
	//BC2_TYPELESS = 73,
	//BC2_UNORM = 74,
	//BC2_UNORM_SRGB = 75,
	//BC3_TYPELESS = 76,
	//BC3_UNORM = 77,
	//BC3_UNORM_SRGB = 78,
	//BC4_TYPELESS = 79,
	//BC4_UNORM = 80,
	//BC4_SNORM = 81,
	//BC5_TYPELESS = 82,
	//BC5_UNORM = 83,
	//BC5_SNORM = 84,

	//B5G6R5_UNORM = 85,
	//B5G5R5A1_UNORM = 86,
	//B8G8R8A8_UNORM = 87,
	//B8G8R8X8_UNORM = 88,
	//R10G10B10_XR_BIAS_A2_UNORM = 89,
	//B8G8R8A8_TYPELESS = 90,
	//B8G8R8A8_UNORM_SRGB = 91,
	//B8G8R8X8_TYPELESS = 92,
	//B8G8R8X8_UNORM_SRGB = 93,
	//BC6H_TYPELESS = 94,
	//BC6H_UF16 = 95,
	//BC6H_SF16 = 96,
	//BC7_TYPELESS = 97,
	//BC7_UNORM = 98,
	//BC7_UNORM_SRGB = 99,
	//AYUV = 100,
	//Y410 = 101,
	//Y416 = 102,
	//NV12 = 103,
	//P010 = 104,
	//P016 = 105,
	//_420_OPAQUE = 106,
	//YUY2 = 107,
	//Y210 = 108,
	//Y216 = 109,
	//NV11 = 110,
	//AI44 = 111,
	//IA44 = 112,
	//P8 = 113,
	//A8P8 = 114,
	//B4G4R4A4_UNORM = 115,
	//P208 = 130,
	//V208 = 131,
	//V408 = 132,
};

enum class ePrimitiveTopology {
	UNDEFINED = 0,
	POINTLIST = 1,
	LINELIST = 2,
	LINESTRIP = 3,
	TRIANGLELIST = 4,
	TRIANGLESTRIP = 5,
	LINELIST_ADJ = 10,
	LINESTRIP_ADJ = 11,
	TRIANGLELIST_ADJ = 12,
	TRIANGLESTRIP_ADJ = 13,
	PATCHLIST_1 = 33,
	PATCHLIST_2 = 34,
	PATCHLIST_3 = 35,
	PATCHLIST_4 = 36,
	PATCHLIST_5 = 37,
	PATCHLIST_6 = 38,
	PATCHLIST_7 = 39,
	PATCHLIST_8 = 40,
	PATCHLIST_9 = 41,
	PATCHLIST_10 = 42,
	PATCHLIST_11 = 43,
	PATCHLIST_12 = 44,
	PATCHLIST_13 = 45,
	PATCHLIST_14 = 46,
	PATCHLIST_15 = 47,
	PATCHLIST_16 = 48,
	PATCHLIST_17 = 49,
	PATCHLIST_18 = 50,
	PATCHLIST_19 = 51,
	PATCHLIST_20 = 52,
	PATCHLIST_21 = 53,
	PATCHLIST_22 = 54,
	PATCHLIST_23 = 55,
	PATCHLIST_24 = 56,
	PATCHLIST_25 = 57,
	PATCHLIST_26 = 58,
	PATCHLIST_27 = 59,
	PATCHLIST_28 = 60,
	PATCHLIST_29 = 61,
	PATCHLIST_30 = 62,
	PATCHLIST_31 = 63,
	PATCHLIST_32 = 64,
};

enum class ePrimitiveTopologyType {
	UNDEFINED = 0,
	POINT = 1,
	LINE = 2,
	TRIANGLE = 3,
	PATCH = 4
};

enum class eTriangleStripCutIndex {
	DISABLED = 0,
	FFFFh = 1,
	FFFFFFFFh = 2
};

enum class eCommandListType {
	COPY,
	COMPUTE,
	GRAPHICS,
	BUNDLE,
};


enum class eCommandQueuePriority {
	NORMAL,
	HIGH,
};


enum class eDescriptorHeapType {
	CBV_SRV_UAV,
	SAMPLER,
	RTV,
	DSV,
};


enum class eHeapType {
	DEFAULT,
	UPLOAD,
	READBACK,
	CUSTOM,
};

enum class eCpuPageProperty {
	UNKNOWN,
	NOT_AVAILABLE,
	WRITE_COMBINE,
	WRITE_BACK,
};

enum class eMemoryPool {
	UNKNOWN,
	HOST,
	DEDICATED,
};

enum class eTextueDimension {
	ONE,
	TWO,
	THREE,
};

enum class eTextureLayout {
	UNKNOWN,
	ROW_MAJOR,
	UNDEFINED_SWIZZLE,
	STANDARD_SWIZZLE,
};


enum class eResourceType {
	TEXTURE,
	BUFFER,
};

enum class eBlendOperand {
	ZERO,
	ONE,

	SHADER_OUT,
	INV_SHADER_OUT,
	SHADER_ALPHA,
	INV_SHADER_ALPHA,

	TARGET_OUT,
	INV_TARGET_OUT,
	TARGET_ALPHA,
	INV_TARGET_ALPHA,

	SHADER_ALPHA_SAT,
	BLEND_FACTOR,
	INV_BLEND_FACTOR,
};

enum class eBlendOperation {
	ADD,
	SUBTRACT,
	REVERSE_SUBTRACT,
	MIN,
	MAX,
};

enum class eBlendLogicOperation {
	CLEAR,
	SET,
	COPY,
	COPY_INVERTED,
	NOOP,
	INVERT,
	AND,
	NAND,
	OR,
	NOR,
	XOR,
	EQUIV,
	AND_REVERSE,
	AND_INVERTED,
	OR_REVERSE,
	OR_INVERTED,
};

enum class eFillMode {
	WIREFRAME,
	SOLID,
};

enum class eCullMode {
	DRAW_ALL,
	DRAW_CW,
	DRAW_CCW,
};

enum class eConservativeRasterizationMode {
	ON,
	OFF,
};

enum class eComparisonFunction {
	NEVER,
	LESS,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
	EQUAL,
	NOT_EQUAL,
	ALWAYS,
};

enum class eStencilOp {
	KEEP,
	ZERO,
	REPLACE,
	INCR_SAT,
	DECR_SAT,
	INCR_WRAP,
	DECR_WRAP,
	INVERT,
};

enum class eInputClassification {
	VERTEX_DATA,
	INSTANCE_DATA,
};


enum class eShaderVisiblity {
	ALL,
	VERTEX,
	GEOMETRY,
	HULL,
	DOMAIN,
	PIXEL,
};

enum class eTextureAddressMode {
	WRAP,
	MIRROR,
	BORDER,
	CLAMP,
	MIRROR_ONE,
};

enum class eTextureFilterMode {
	MIN_MAG_MIP_POINT = 0,
	MIN_MAG_POINT_MIP_LINEAR = 0x1,
	MIN_POINT_MAG_LINEAR_MIP_POINT = 0x4,
	MIN_POINT_MAG_MIP_LINEAR = 0x5,
	MIN_LINEAR_MAG_MIP_POINT = 0x10,
	MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x11,
	MIN_MAG_LINEAR_MIP_POINT = 0x14,
	MIN_MAG_MIP_LINEAR = 0x15,
	ANISOTROPIC = 0x55,
	COMPARISON_MIN_MAG_MIP_POINT = 0x80,
	COMPARISON_MIN_MAG_POINT_MIP_LINEAR = 0x81,
	COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x84,
	COMPARISON_MIN_POINT_MAG_MIP_LINEAR = 0x85,
	COMPARISON_MIN_LINEAR_MAG_MIP_POINT = 0x90,
	COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x91,
	COMPARISON_MIN_MAG_LINEAR_MIP_POINT = 0x94,
	COMPARISON_MIN_MAG_MIP_LINEAR = 0x95,
	COMPARISON_ANISOTROPIC = 0xd5,
	MINIMUM_MIN_MAG_MIP_POINT = 0x100,
	MINIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x101,
	MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x104,
	MINIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x105,
	MINIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x110,
	MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x111,
	MINIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x114,
	MINIMUM_MIN_MAG_MIP_LINEAR = 0x115,
	MINIMUM_ANISOTROPIC = 0x155,
	MAXIMUM_MIN_MAG_MIP_POINT = 0x180,
	MAXIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x181,
	MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x184,
	MAXIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x185,
	MAXIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x190,
	MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x191,
	MAXIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x194,
	MAXIMUM_MIN_MAG_MIP_LINEAR = 0x195,
	MAXIMUM_ANISOTROPIC = 0x1d5
};

enum class eTextureBorderColor {
	TRANSPARENT_BLACK,
	OPAQUE_BLACK,
	OPAQUE_WHITE,
};


enum class eDsvDimension {
	UNKNOWN = 0,
	TEXTURE1D = 1,
	TEXTURE1DARRAY = 2,
	TEXTURE2D = 3,
	TEXTURE2DARRAY = 4,
	TEXTURE2DMS = 5,
	TEXTURE2DMSARRAY = 6,
};

enum class eRtvDimension {
	UNKNOWN = 0,
	BUFFER = 1,
	TEXTURE1D = 2,
	TEXTURE1DARRAY = 3,
	TEXTURE2D = 4,
	TEXTURE2DARRAY = 5,
	TEXTURE2DMS = 6,
	TEXTURE2DMSARRAY = 7,
	TEXTURE3D = 8
};


enum class eSrvDimension {
	UNKNOWN = 0,
	BUFFER = 1,
	TEXTURE1D = 2,
	TEXTURE1DARRAY = 3,
	TEXTURE2D = 4,
	TEXTURE2DARRAY = 5,
	TEXTURE2DMS = 6,
	TEXTURE2DMSARRAY = 7,
	TEXTURE3D = 8,
	TEXTURECUBE = 9,
	TEXTURECUBEARRAY = 10
};

enum class eUavDimension {
	UNKNOWN = 0,
	BUFFER = 1,
	TEXTURE1D = 2,
	TEXTURE1DARRAY = 3,
	TEXTURE2D = 4,
	TEXTURE2DARRAY = 5,
	TEXTURE3D = 8
};

enum class eResourceBarrierSplit {
	NORMAL,
	BEGIN,
	END,
};

enum class eResourceBarrierType {
	TRANSITION,
	ALIASING,
	UAV,
};

//------------------------------------------------------------------------------
// Bitflag enumerations
//------------------------------------------------------------------------------


namespace bitflag_enum_impl {


	struct eShaderCompileFlags_Base {
		enum EnumT {
			DEBUG = (1 << 0),
			NO_OPTIMIZATION = (1 << 1),
			ROW_MAJOR_MATRICES = (1 << 2),
			COLUMN_MAJOR_MATRICES = (1 << 3),
			FORCE_IEEE = (1 << 4),
			WARNINGS_AS_ERRORS = (1 << 5),
			OPTIMIZATION_LOW = (1 << 6),
			OPTIMIZATION_MEDIUM = (1 << 7),
			OPTIMIZATION_HIGH = (1 << 8),
		};
	};


	struct eHeapFlags_Base {
		enum EnumT {
			NONE = 0,
			SHARED = 0x1,
			DENY_BUFFERS = 0x4,
			ALLOW_DISPLAY = 0x8,
			SHARED_CROSS_ADAPTER = 0x20,
			DENY_RT_DS_TEXTURES = 0x40,
			DENY_NON_RT_DS_TEXTURES = 0x80,
			ALLOW_ALL_BUFFERS_AND_TEXTURES = 0,
			ALLOW_ONLY_BUFFERS = 0xc0,
			ALLOW_ONLY_NON_RT_DS_TEXTURES = 0x44,
			ALLOW_ONLY_RT_DS_TEXTURES = 0x84
		};
	};

	struct eResourceState_Base {
		enum EnumT {
			COMMON = 0,
			VERTEX_AND_CONSTANT_BUFFER = 0x1,
			INDEX_BUFFER = 0x2,
			RENDER_TARGET = 0x4,
			UNORDERED_ACCESS = 0x8,
			DEPTH_WRITE = 0x10,
			DEPTH_READ = 0x20,
			NON_PIXEL_SHADER_RESOURCE = 0x40,
			PIXEL_SHADER_RESOURCE = 0x80,
			STREAM_OUT = 0x100,
			INDIRECT_ARGUMENT = 0x200,
			COPY_DEST = 0x400,
			COPY_SOURCE = 0x800,
			RESOLVE_DEST = 0x1000,
			RESOLVE_SOURCE = 0x2000,
			GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
			PRESENT = 0,
			PREDICATION = 0x200
		};
	};

	struct eResourceFlags_Base {
		enum EnumT {
			NONE = 0,
			ALLOW_RENDER_TARGET = 0x1,
			ALLOW_DEPTH_STENCIL = 0x2,
			ALLOW_UNORDERED_ACCESS = 0x4,
			DENY_SHADER_RESOURCE = 0x8,
			ALLOW_CROSS_ADAPTER = 0x10,
			ALLOW_SIMULTANEOUS_ACCESS = 0x20
		};
	};

	struct eColorMask_Base {
		enum EnumT {
			RED = 1,
			GREEN = 2,
			BLUE = 4,
			ALPHA = 8,
			ALL = (RED | GREEN | BLUE | ALPHA),
		};
	};

	struct eDsvFlags_Base {
		enum EnumT {
			NONE = 0,
			READ_ONLY_DEPTH = 0x1,
			READ_ONLY_STENCIL = 0x2,
		};
	};

} // namespace bitflag_enum_impl


using eHeapFlags = EnumFlag_Helper<bitflag_enum_impl::eHeapFlags_Base>;
using eResourceState = EnumFlag_Helper<bitflag_enum_impl::eResourceState_Base>;
using eResourceFlags = EnumFlag_Helper<bitflag_enum_impl::eResourceFlags_Base>;
using eColorMask = EnumFlag_Helper<bitflag_enum_impl::eColorMask_Base>;
using eDsvFlags = EnumFlag_Helper<bitflag_enum_impl::eDsvFlags_Base>;
using eShaderCompileFlags = EnumFlag_Helper<bitflag_enum_impl::eShaderCompileFlags_Base>;



//------------------------------------------------------------------------------
// Not rendering structures
//------------------------------------------------------------------------------

struct AdapterInfo {
	unsigned adapterId;
	std::string name;
	unsigned vendorId;
	unsigned deviceId;
	size_t dedicatedVideoMemory;
	size_t dedicatedSystemMemory;
	size_t sharedSystemMemory;
	bool isSoftwareAdapter;
};

struct SwapChainDesc {
	unsigned width;
	unsigned height;
	eFormat format;
	unsigned multisampleCount;
	unsigned multiSampleQuality;
	unsigned numBuffers;
	NativeWindowHandle targetWindow;
	bool isFullScreen;
};


struct ShaderProgramBinary {
	std::vector<uint8_t> data;
};


struct ShaderMacroDefinition {
	std::string name;
	std::string value;
};


struct MemoryRange {
	size_t begin;
	size_t end;
};


//------------------------------------------------------------------------------
// Rendering structures
//------------------------------------------------------------------------------



// todo: operator++, += for traversing heap like iterator?
//		ctor as obj(Heap*, index)
struct DescriptorHandle {
	bool operator==(const DescriptorHandle& rhs) const = default;
	bool operator!=(const DescriptorHandle& rhs) const = default;

	void* cpuAddress = nullptr;
	void* gpuAddress = nullptr;
};


struct Viewport {
	float topLeftX = 0;
	float topLeftY = 0;
	float width = 0;
	float height = 0;
	float minDepth = 0;
	float maxDepth = 1;
};

struct HeapProperties {
	eHeapType type = eHeapType::DEFAULT;
	eCpuPageProperty cpuPageProperty = eCpuPageProperty::UNKNOWN;
	eMemoryPool pool = eMemoryPool::UNKNOWN;
};

struct BufferDesc {
	uint64_t sizeInBytes = 0;
};

struct TextureDesc {
	static constexpr uint16_t ALL_MIPLEVELS = 0;

	eTextueDimension dimension = eTextueDimension::TWO;
	uint64_t alignment = 0;
	uint64_t width = 0;
	uint32_t height = 0;
	uint16_t depthOrArraySize = 0;
	uint16_t mipLevels = 0;
	eFormat format = eFormat::UNKNOWN;
	eTextureLayout layout = eTextureLayout::UNKNOWN;
	eResourceFlags flags = eResourceFlags::NONE;
	uint32_t multisampleCount = 1;
	uint32_t multisampleQuality = 0;
};


struct ResourceDesc {
	eResourceType type = eResourceType::BUFFER;

	// C++ won't let it be a union because members are not POD types technically... Fuck that!
	//union {
	TextureDesc textureDesc;
	BufferDesc bufferDesc;
	//};

	static inline ResourceDesc Buffer(uint64_t sizeInBytes);
	static inline ResourceDesc Texture1D(uint64_t width, eFormat format, eResourceFlags flags = eResourceFlags::NONE,
										 uint16_t mipLevels = 1, uint32_t multisampleCount = 1, uint32_t multisampleQuality = 0,
										 uint64_t alignment = 0, eTextureLayout layout = eTextureLayout::UNKNOWN);

	static inline ResourceDesc Texture1DArray(uint64_t width, eFormat format, uint16_t arraySize,
											  eResourceFlags flags = eResourceFlags::NONE,
											  uint16_t mipLevels = 1, uint32_t multisampleCount = 1, uint32_t multisampleQuality = 0,
											  uint64_t alignment = 0, eTextureLayout layout = eTextureLayout::UNKNOWN);

	static inline ResourceDesc Texture2D(uint64_t width, uint32_t height, eFormat format,
										 eResourceFlags flags = eResourceFlags::NONE,
										 uint16_t mipLevels = 1, uint32_t multisampleCount = 1, uint32_t multisampleQuality = 0,
										 uint64_t alignment = 0, eTextureLayout layout = eTextureLayout::UNKNOWN);

	static inline ResourceDesc Texture2DArray(uint64_t width, uint32_t height, eFormat format, uint16_t arraySize,
											  eResourceFlags flags = eResourceFlags::NONE,
											  uint16_t mipLevels = 1, uint32_t multisampleCount = 1, uint32_t multisampleQuality = 0,
											  uint64_t alignment = 0, eTextureLayout layout = eTextureLayout::UNKNOWN);

	static inline ResourceDesc Texture3D(uint64_t width, uint32_t height, uint16_t depth, eFormat format,
										 eResourceFlags flags = eResourceFlags::NONE,
										 uint16_t mipLevels = 1, uint32_t multisampleCount = 1, uint32_t multisampleQuality = 0,
										 uint64_t alignment = 0, eTextureLayout layout = eTextureLayout::UNKNOWN);

	static inline ResourceDesc CubeMap(uint64_t width, uint32_t height, eFormat format,
									   eResourceFlags flags = eResourceFlags::NONE,
									   uint16_t mipLevels = 1, uint32_t multisampleCount = 1, uint32_t multisampleQuality = 0,
									   uint64_t alignment = 0, eTextureLayout layout = eTextureLayout::UNKNOWN);

	static inline ResourceDesc CubeMapArray(uint64_t width, uint32_t height, eFormat format, uint16_t arraySize,
											eResourceFlags flags = eResourceFlags::NONE,
											uint16_t mipLevels = 1, uint32_t multisampleCount = 1, uint32_t multisampleQuality = 0,
											uint64_t alignment = 0, eTextureLayout layout = eTextureLayout::UNKNOWN);
};


struct ClearValue {
	ClearValue() = default;
	ClearValue(eFormat format, float r, float g, float b, float a) : format(format), color(r, g, b, a) {}
	ClearValue(eFormat format, ColorRGBA color) : format(format), color(color) {}
	ClearValue(eFormat format, float depth, uint8_t stencil = 0) : format(format), depthStencil{ depth, stencil } {}
	eFormat format;
	union {
		ColorRGBA color;
		struct {
			float depth;
			uint8_t stencil;
		} depthStencil;
	};
};


// Describes which subresource of a texture should be used or
// how data should be interpreted inside a buffer when treating it as a texture during a copy
struct TextureCopyDesc {
	// to describe texture information inside a buffer:
	eFormat format = eFormat::UNKNOWN;
	uint64_t width = 0;
	uint32_t height = 0;
	uint16_t depth = 0;
	size_t byteOffset = 0;

	// to identify a texture subresource:
	uint32_t subresourceIndex = 0;


	static TextureCopyDesc Texture(uint32_t subresourceIndex) {
		return TextureCopyDesc{ .subresourceIndex = subresourceIndex };
	}

	static TextureCopyDesc Buffer(eFormat format, uint64_t width, uint32_t height, uint16_t depth, size_t byteOffset) {
		return TextureCopyDesc{ .format = format, .width = width, .height = height, .depth = depth, .byteOffset = byteOffset };
	}
};


struct CommandQueueDesc {
	eCommandListType type = eCommandListType::GRAPHICS;
	eCommandQueuePriority priority = eCommandQueuePriority::NORMAL;
	bool enableGpuTimeout = true;
};


struct CommandListDesc {
	ICommandAllocator* allocator = nullptr;
	IPipelineState* initialState = nullptr;
};


struct DescriptorHeapDesc {
	eDescriptorHeapType type = eDescriptorHeapType::CBV_SRV_UAV;
	size_t numDescriptors = 0;
	bool isShaderVisible = false;
};


struct ShaderByteCodeDesc {
	const void* shaderByteCode = nullptr;
	size_t sizeOfByteCode = 0;
};


struct StreamOutputState {
private:
};


struct RenderTargetBlendState {
	bool operator==(const RenderTargetBlendState&) const = default;
	bool operator!=(const RenderTargetBlendState&) const = default;

	bool enableBlending = false;
	bool enableLogicOp = false;

	eBlendOperand shaderColorFactor = eBlendOperand::SHADER_OUT;
	eBlendOperand targetColorFactor = eBlendOperand::ZERO;
	eBlendOperation colorOperation = eBlendOperation::ADD;

	eBlendOperand shaderAlphaFactor = eBlendOperand::SHADER_OUT;
	eBlendOperand targetAlphaFactor = eBlendOperand::ZERO;
	eBlendOperation alphaOperation = eBlendOperation::ADD;

	eColorMask mask = eColorMask::ALL;

	eBlendLogicOperation logicOperation = eBlendLogicOperation::NOOP;
};


struct BlendState {
	BlendState() : singleTarget(multiTarget[0]) {}
	BlendState& operator=(const BlendState& other) {
		if (this == &other) {
			return *this;
		}

		alphaToCoverage = other.alphaToCoverage;
		independentBlending = other.independentBlending;
		memcpy(multiTarget, other.multiTarget, sizeof(multiTarget));

		return *this;
	}
	bool alphaToCoverage = false;
	bool independentBlending = false;
	RenderTargetBlendState multiTarget[8];
	RenderTargetBlendState& singleTarget;
};


struct RasterizerState {
	eFillMode fillMode = eFillMode::SOLID;
	eCullMode cullMode = eCullMode::DRAW_ALL;
	int depthBias = 0;
	float depthBiasClamp = 0;
	float slopeScaledDepthBias = 0;
	bool depthClipEnabled = false;
	bool multisampleEnabled = false;
	bool lineAntialiasingEnabled = false;
	unsigned forcedSampleCount = 0;
	eConservativeRasterizationMode conservativeRasterization = eConservativeRasterizationMode::OFF;
};


struct InputElementDesc {
	const char* semanticName = nullptr;
	unsigned semanticIndex = 0;
	eFormat format = eFormat::UNKNOWN;
	unsigned inputSlot = 0;
	unsigned offset = 0;
	eInputClassification classifiacation = eInputClassification::VERTEX_DATA;
	unsigned instanceDataStepRate = 0;
};

struct InputLayout {
	unsigned numElements = 0;
	InputElementDesc* elements = nullptr;
};

struct DepthStencilState {
	struct FaceOperations {
		eStencilOp stencilOpOnStencilFail = eStencilOp::KEEP;
		eStencilOp stencilOpOnDepthFail = eStencilOp::KEEP;
		eStencilOp stencilOpOnPass = eStencilOp::KEEP;
		eComparisonFunction stencilFunc = eComparisonFunction::ALWAYS;
	};

	bool enableDepthTest = false;
	bool enableDepthStencilWrite = false;
	eComparisonFunction depthFunc = eComparisonFunction::LESS;
	bool enableStencilTest = false;
	uint8_t stencilReadMask = 0;
	uint8_t stencilWriteMask = 0;

	FaceOperations cwFace, ccwFace;
};


/// <summary> Describes the fixed function parts of the GPU pipeline. </summary>
/// <remarks> Values are filled with sensible defaults. </remarks>
struct GraphicsPipelineStateDesc {
	IRootSignature* rootSignature = nullptr;

	ShaderByteCodeDesc vs;
	ShaderByteCodeDesc gs;
	ShaderByteCodeDesc hs;
	ShaderByteCodeDesc ds;
	ShaderByteCodeDesc ps;

	StreamOutputState streamOutput;
	RasterizerState rasterization;
	DepthStencilState depthStencilState;
	BlendState blending;
	unsigned blendSampleMask = 0xFFFFFFFF;

	InputLayout inputLayout;
	ePrimitiveTopologyType primitiveTopologyType = ePrimitiveTopologyType::UNDEFINED;
	eTriangleStripCutIndex triangleStripCutIndex = eTriangleStripCutIndex::DISABLED;

	unsigned numRenderTargets = 1;
	eFormat renderTargetFormats[8] = { eFormat::UNKNOWN, eFormat::UNKNOWN, eFormat::UNKNOWN, eFormat::UNKNOWN, eFormat::UNKNOWN, eFormat::UNKNOWN, eFormat::UNKNOWN, eFormat::UNKNOWN };
	eFormat depthStencilFormat = eFormat::UNKNOWN;
	unsigned multisampleCount = 1;
	unsigned multisampleQuality = 0;

	bool addDebugInfo = false;
};

struct ComputePipelineStateDesc {
	IRootSignature* rootSignature = nullptr;
	ShaderByteCodeDesc cs = {};
	bool addDebugInfo = false;
};

struct DescriptorRange {
	enum eType {
		CBV,
		SRV,
		UAV,
		SAMPLER,
	};

	eType type = CBV;
	unsigned numDescriptors = 0;
	unsigned baseShaderRegister = 0;
	unsigned registerSpace = 0;
	unsigned offsetFromTableStart = OFFSET_APPEND;

	static constexpr auto OFFSET_APPEND = std::numeric_limits<unsigned>::max();
};

struct RootDescriptorTable {
	std::vector<DescriptorRange> ranges;
};

struct RootConstant {
	RootConstant() = default;
	RootConstant(unsigned shaderRegister, unsigned registerSpace, unsigned numConstant)
		: shaderRegister(shaderRegister), registerSpace(registerSpace), numConstants(numConstants) {}
	unsigned shaderRegister;
	unsigned registerSpace;
	unsigned numConstants;
};

struct RootDescriptor {
	RootDescriptor() = default;
	RootDescriptor(unsigned shaderRegister, unsigned registerSpace)
		: shaderRegister(shaderRegister), registerSpace(registerSpace) {}
	unsigned shaderRegister;
	unsigned registerSpace;
};


struct RootParameterDesc {
	enum eType {
		CONSTANT,
		CBV,
		SRV,
		UAV,
		DESCRIPTOR_TABLE,
		UNITIALIZED,
	};

	// Special ctors and dtor to manage unions
	RootParameterDesc() : m_type(UNITIALIZED), type(m_type), shaderVisibility(m_shaderVisibility), m_shaderVisibility(eShaderVisiblity::ALL){};
	RootParameterDesc(const RootParameterDesc& rhs) : m_type(rhs.m_type), type(m_type), shaderVisibility(m_shaderVisibility) {
		m_shaderVisibility = rhs.m_shaderVisibility;
		switch (m_type) {
			case CONSTANT:
				new (&constant) RootConstant(rhs.constant);
				break;
			case CBV:
			case SRV:
			case UAV:
				new (&descriptor) RootDescriptor(rhs.descriptor);
				break;
			case DESCRIPTOR_TABLE:
				new (&descriptorTable) RootDescriptorTable(rhs.descriptorTable);
				break;
		}
	}
	RootParameterDesc(RootParameterDesc&& rhs) : m_type(rhs.m_type), type(m_type), shaderVisibility(m_shaderVisibility) {
		m_shaderVisibility = rhs.m_shaderVisibility;
		switch (m_type) {
			case CONSTANT:
				new (&constant) RootConstant(std::move(rhs.constant));
				break;
			case CBV:
			case SRV:
			case UAV:
				new (&descriptor) RootDescriptor(std::move(rhs.descriptor));
				break;
			case DESCRIPTOR_TABLE:
				new (&descriptorTable) RootDescriptorTable(std::move(rhs.descriptorTable));
				break;
		}
	}
	RootParameterDesc& operator=(const RootParameterDesc& rhs) {
		Destruct();
		new (this) RootParameterDesc(rhs);
		return *this;
	}
	RootParameterDesc& operator=(RootParameterDesc&& rhs) noexcept {
		Destruct();
		new (this) RootParameterDesc(std::move(rhs));
		return *this;
	}
	~RootParameterDesc() {
		Destruct();
	}

	// Access to internal parameters.
	const eType& type;
	const eShaderVisiblity& shaderVisibility;

	template <eType Type>
	const typename std::conditional<Type == CONSTANT, RootConstant,
									typename std::conditional<Type == CBV || Type == SRV || Type == UAV, RootDescriptor,
															  typename std::conditional<Type == DESCRIPTOR_TABLE, RootDescriptorTable, void>::type>::type>::type&
	As() const {};
	template <eType Type>
	typename std::conditional<Type == CONSTANT, RootConstant,
							  typename std::conditional<Type == CBV || Type == SRV || Type == UAV, RootDescriptor,
														typename std::conditional<Type == DESCRIPTOR_TABLE, RootDescriptorTable, void>::type>::type>::type&
	As(){};


	// Constructors to initialize object.
	static inline RootParameterDesc Constant(unsigned numConstants, unsigned shaderRegister, unsigned registerSpace = 0,
											 eShaderVisiblity shaderVisibility = eShaderVisiblity::ALL);

	static inline RootParameterDesc Cbv(unsigned shaderRegister, unsigned registerSpace = 0,
										eShaderVisiblity shaderVisibility = eShaderVisiblity::ALL);

	static inline RootParameterDesc Srv(unsigned shaderRegister, unsigned registerSpace = 0,
										eShaderVisiblity shaderVisibility = eShaderVisiblity::ALL);

	static inline RootParameterDesc Uav(unsigned shaderRegister, unsigned registerSpace = 0,
										eShaderVisiblity shaderVisibility = eShaderVisiblity::ALL);

	static inline RootParameterDesc DescriptorTable(unsigned numDescriptorRanges, DescriptorRange* descriptorRanges,
													eShaderVisiblity shaderVisibility = eShaderVisiblity::ALL);
	static inline RootParameterDesc DescriptorTable(std::vector<DescriptorRange> descriptorRanges,
													eShaderVisiblity shaderVisibility = eShaderVisiblity::ALL);
	static inline RootParameterDesc DescriptorTable(eShaderVisiblity shaderVisibility = eShaderVisiblity::ALL);

private:
	void Destruct() {
		switch (m_type) {
			case CONSTANT:
				constant.~RootConstant();
				break;
			case CBV:
			case SRV:
			case UAV:
				descriptor.~RootDescriptor();
				break;
			case DESCRIPTOR_TABLE:
				descriptorTable.~RootDescriptorTable();
				break;
		}
	}

	eType m_type;
	union {
		RootDescriptorTable descriptorTable;
		RootDescriptor descriptor;
		RootConstant constant;
	};
	eShaderVisiblity m_shaderVisibility;
};

struct StaticSamplerDesc {
	eTextureFilterMode filter = eTextureFilterMode::ANISOTROPIC;
	eTextureAddressMode addressU = eTextureAddressMode::WRAP;
	eTextureAddressMode addressV = eTextureAddressMode::WRAP;
	eTextureAddressMode addressW = eTextureAddressMode::WRAP;
	float mipLevelBias = 0;
	unsigned maxAnisotropy = 16;
	eComparisonFunction compareFunc = eComparisonFunction::LESS_EQUAL;
	eTextureBorderColor border = eTextureBorderColor::OPAQUE_WHITE;
	float minMipLevel = 0;
	float maxMipLevel = std::numeric_limits<float>::max();
	unsigned shaderRegister = 0;
	unsigned registerSpace = 0;
	eShaderVisiblity shaderVisibility = eShaderVisiblity::ALL;
};

struct RootSignatureDesc {
	std::vector<RootParameterDesc> rootParameters;
	std::vector<StaticSamplerDesc> staticSamplers;
};



// buffer views

struct ConstantBufferViewDesc {
	const void* gpuVirtualAddress;
	size_t sizeInBytes;
};


struct DsvTexture1D {
	unsigned firstMipLevel;
};
struct DsvTexture1DArray {
	unsigned firstMipLevel;
	unsigned firstArrayElement;
	unsigned activeArraySize;
};
struct DsvTexture2D {
	unsigned firstMipLevel;
};
struct DsvTexture2DArray {
	unsigned firstMipLevel;
	unsigned firstArrayElement;
	unsigned activeArraySize;
};
struct DsvTextureMultisampled2D {
	// empty on purpose //
};
struct DsvTextureMultisampled2DArray {
	unsigned firstArrayElement;
	unsigned activeArraySize;
};


struct DepthStencilViewDesc {
	eFormat format = eFormat::UNKNOWN;
	eDsvDimension dimension = eDsvDimension::UNKNOWN;
	eDsvFlags flags = eDsvFlags::NONE;
	union {
		DsvTexture1D tex1D;
		DsvTexture1DArray tex1DArray;
		DsvTexture2D tex2D;
		DsvTexture2DArray tex2DArray;
		DsvTextureMultisampled2D texMS2D;
		DsvTextureMultisampled2DArray texMS2DArray;
	};
};



struct RtvBuffer {
	size_t firstElement;
	unsigned numElements;
};
struct RtvTexture1D {
	unsigned firstMipLevel;
};
struct RtvTexture1DArray {
	unsigned firstMipLevel;
	unsigned firstArrayElement;
	unsigned activeArraySize;
};
struct RtvTexture2D {
	unsigned firstMipLevel;
	unsigned planeIndex; // this has to do with format, e.g. D24_S8 has 2 planes
};
struct RtvTexture2DArray {
	unsigned firstMipLevel;
	unsigned firstArrayElement;
	unsigned activeArraySize;
	unsigned planeIndex;
};
struct RtvTextureMultisampled2D {
	// empty on purpose //
};
struct RtvTextureMultisampled2DArray {
	unsigned firstArrayElement;
	unsigned activeArraySize;
};
struct RtvTexture3D {
	unsigned firstMipLevel;
	unsigned firstDepthIndex;
	unsigned numDepthLevels;
};

struct RenderTargetViewDesc {
	eFormat format = eFormat::UNKNOWN;
	eRtvDimension dimension = eRtvDimension::UNKNOWN;
	union {
		RtvBuffer buffer;
		RtvTexture1D tex1D;
		RtvTexture1DArray tex1DArray;
		RtvTexture2D tex2D;
		RtvTexture2DArray tex2DArray;
		RtvTextureMultisampled2D texMS2D;
		RtvTextureMultisampled2DArray texMS2DArray;
		RtvTexture3D tex3D;
	};
};



struct SrvBuffer {
	size_t firstElement;
	unsigned numElements;
	unsigned structureStrideInBytes;
	bool isRaw;
};
struct SrvTexture1D {
	unsigned mostDetailedMip;
	unsigned numMipLevels;
	float mipLevelClamping;
};
struct SrvTexture1DArray {
	unsigned mostDetailedMip;
	unsigned numMipLevels;
	float mipLevelClamping;

	unsigned firstArrayElement;
	unsigned activeArraySize;
};
struct SrvTexture2D {
	unsigned mostDetailedMip;
	unsigned numMipLevels;
	float mipLevelClamping;
	unsigned planeIndex; // this has to do with format, e.g. D24_S8 has 2 planes
};
struct SrvTexture2DArray {
	unsigned mostDetailedMip;
	unsigned numMipLevels;
	float mipLevelClamping;
	unsigned planeIndex; // this has to do with format, e.g. D24_S8 has 2 planes

	unsigned firstArrayElement;
	unsigned activeArraySize;
};
struct SrvTextureMultisampled2D {
	// empty on purpose //
};
struct SrvTextureMultisampled2DArray {
	unsigned firstArrayElement;
	unsigned activeArraySize;
};
struct SrvTexture3D {
	unsigned mostDetailedMip;
	unsigned numMipLevels;
	float mipLevelClamping;
};
struct SrvTextureCube {
	unsigned mostDetailedMip;
	unsigned numMipLevels;
	float mipLevelClamping;
};
struct SrvTextureCubeArray {
	unsigned mostDetailedMip;
	unsigned numMipLevels;
	float mipLevelClamping;
	unsigned indexOfFirst2DTex;
	unsigned numCubes;
};

struct ShaderResourceViewDesc {
	eFormat format = eFormat::UNKNOWN;
	eSrvDimension dimension = eSrvDimension::UNKNOWN;

	union {
		SrvBuffer buffer;
		SrvTexture1D tex1D;
		SrvTexture1DArray tex1DArray;
		SrvTexture2D tex2D;
		SrvTexture2DArray tex2DArray;
		SrvTextureMultisampled2D texMS2D;
		SrvTextureMultisampled2DArray texMS2DArray;
		SrvTexture3D tex3D;
		SrvTextureCube texCube;
		SrvTextureCubeArray texCubeArray;
	};
};


struct UavBuffer {
	bool raw;

	unsigned firstElement;
	unsigned numElements;
	unsigned elementStride;

	unsigned countOffset;
};

struct UavTexture1D {
	unsigned mipLevel;
};

struct UavTexture1DArray {
	unsigned mipLevel;
	unsigned firstArrayElement;
	unsigned activeArraySize;
};

struct UavTexture2D {
	unsigned mipLevel;
	unsigned planeIndex;
};

struct UavTexture2DArray {
	unsigned mipLevel;
	unsigned planeIndex;

	unsigned firstArrayElement;
	unsigned activeArraySize;
};

struct UavTexture3D {
	unsigned mipLevel;

	unsigned firstDepthLayer;
	unsigned depthSize;
};

struct UnorderedAccessViewDesc {
	eFormat format = eFormat::UNKNOWN;
	eUavDimension dimension = eUavDimension::UNKNOWN;
	union {
		UavBuffer buffer;
		UavTexture1D tex1D;
		UavTexture1DArray tex1DArray;
		UavTexture2D tex2D;
		UavTexture2DArray tex2DArray;
		UavTexture3D tex3D;
	};
};



struct ResourceBarrierTag {};

struct TransitionBarrier : public ResourceBarrierTag {
	IResource* resource = nullptr;
	unsigned subResource = 0;
	eResourceState beforeState = eResourceState::COMMON;
	eResourceState afterState = eResourceState::COMMON;
	eResourceBarrierSplit splitMode = eResourceBarrierSplit::NORMAL;
};

static constexpr auto ALL_SUBRESOURCES = std::numeric_limits<unsigned>::max();

struct UavBarrier : public ResourceBarrierTag {
	IResource* resource = nullptr;
};

struct ResourceBarrier {
	eResourceBarrierType type;
	union {
		TransitionBarrier transition;
		UavBarrier uav;
	};
	ResourceBarrier() {}
	ResourceBarrier(const ResourceBarrier& rhs) {
		memcpy(this, &rhs, sizeof(*this));
	}
	ResourceBarrier(const TransitionBarrier& rhs) {
		type = eResourceBarrierType::TRANSITION;
		transition = rhs;
	}
	ResourceBarrier(const UavBarrier& rhs) {
		type = eResourceBarrierType::UAV;
		uav = rhs;
	}

	ResourceBarrier& operator=(const ResourceBarrier& rhs) {
		memcpy(this, &rhs, sizeof(*this));
		return *this;
	}

	ResourceBarrier& operator=(const TransitionBarrier& rhs) {
		type = eResourceBarrierType::TRANSITION;
		transition = rhs;
		return *this;
	}
	ResourceBarrier& operator=(const UavBarrier& rhs) {
		type = eResourceBarrierType::UAV;
		uav = rhs;
		return *this;
	}
};


//------------------------------------------------------------------------------
// User helper functions
//------------------------------------------------------------------------------

inline ResourceDesc ResourceDesc::Buffer(uint64_t sizeInBytes) {
	ResourceDesc desc;
	desc.type = eResourceType::BUFFER;
	desc.bufferDesc.sizeInBytes = sizeInBytes;
	return desc;
}

inline ResourceDesc ResourceDesc::Texture1D(uint64_t width, eFormat format,
											eResourceFlags flags,
											uint16_t mipLevels, uint32_t multisampleCount, uint32_t multisampleQuality,
											uint64_t alignment, eTextureLayout layout) {
	ResourceDesc desc;
	desc.type = eResourceType::TEXTURE;

	desc.textureDesc.dimension = eTextueDimension::ONE;
	desc.textureDesc.format = format;
	desc.textureDesc.width = width;
	desc.textureDesc.height = 1;
	desc.textureDesc.depthOrArraySize = 1;

	desc.textureDesc.flags = flags;
	desc.textureDesc.mipLevels = mipLevels;
	desc.textureDesc.multisampleCount = multisampleCount;
	desc.textureDesc.multisampleQuality = multisampleQuality;
	desc.textureDesc.alignment = alignment;
	desc.textureDesc.layout = layout;

	return desc;
}

inline ResourceDesc ResourceDesc::Texture1DArray(uint64_t width, eFormat format, uint16_t arraySize,
												 eResourceFlags flags,
												 uint16_t mipLevels, uint32_t multisampleCount, uint32_t multisampleQuality,
												 uint64_t alignment, eTextureLayout layout) {
	ResourceDesc desc;
	desc.type = eResourceType::TEXTURE;

	desc.textureDesc.dimension = eTextueDimension::ONE;
	desc.textureDesc.format = format;
	desc.textureDesc.width = width;
	desc.textureDesc.height = 1;
	desc.textureDesc.depthOrArraySize = arraySize;

	desc.textureDesc.flags = flags;
	desc.textureDesc.mipLevels = mipLevels;
	desc.textureDesc.multisampleCount = multisampleCount;
	desc.textureDesc.multisampleQuality = multisampleQuality;
	desc.textureDesc.alignment = alignment;
	desc.textureDesc.layout = layout;

	return desc;
}

inline ResourceDesc ResourceDesc::Texture2D(uint64_t width, uint32_t height, eFormat format,
											eResourceFlags flags,
											uint16_t mipLevels, uint32_t multisampleCount, uint32_t multisampleQuality,
											uint64_t alignment, eTextureLayout layout) {
	ResourceDesc desc;
	desc.type = eResourceType::TEXTURE;

	desc.textureDesc.dimension = eTextueDimension::TWO;
	desc.textureDesc.format = format;
	desc.textureDesc.width = width;
	desc.textureDesc.height = height;
	desc.textureDesc.depthOrArraySize = 1;

	desc.textureDesc.flags = flags;
	desc.textureDesc.mipLevels = mipLevels;
	desc.textureDesc.multisampleCount = multisampleCount;
	desc.textureDesc.multisampleQuality = multisampleQuality;
	desc.textureDesc.alignment = alignment;
	desc.textureDesc.layout = layout;

	return desc;
}
inline ResourceDesc ResourceDesc::Texture2DArray(uint64_t width, uint32_t height, eFormat format, uint16_t arraySize,
												 eResourceFlags flags,
												 uint16_t mipLevels, uint32_t multisampleCount, uint32_t multisampleQuality,
												 uint64_t alignment, eTextureLayout layout) {
	ResourceDesc desc;
	desc.type = eResourceType::TEXTURE;

	desc.textureDesc.dimension = eTextueDimension::TWO;
	desc.textureDesc.format = format;
	desc.textureDesc.width = width;
	desc.textureDesc.height = height;
	desc.textureDesc.depthOrArraySize = arraySize;

	desc.textureDesc.flags = flags;
	desc.textureDesc.mipLevels = mipLevels;
	desc.textureDesc.multisampleCount = multisampleCount;
	desc.textureDesc.multisampleQuality = multisampleQuality;
	desc.textureDesc.alignment = alignment;
	desc.textureDesc.layout = layout;

	return desc;
}

inline ResourceDesc ResourceDesc::Texture3D(uint64_t width, uint32_t height, uint16_t depth, eFormat format,
											eResourceFlags flags,
											uint16_t mipLevels, uint32_t multisampleCount, uint32_t multisampleQuality,
											uint64_t alignment, eTextureLayout layout) {
	ResourceDesc desc;
	desc.type = eResourceType::TEXTURE;

	desc.textureDesc.dimension = eTextueDimension::THREE;
	desc.textureDesc.format = format;
	desc.textureDesc.width = width;
	desc.textureDesc.height = height;
	desc.textureDesc.depthOrArraySize = depth;

	desc.textureDesc.flags = flags;
	desc.textureDesc.mipLevels = mipLevels;
	desc.textureDesc.multisampleCount = multisampleCount;
	desc.textureDesc.multisampleQuality = multisampleQuality;
	desc.textureDesc.alignment = alignment;
	desc.textureDesc.layout = layout;

	return desc;
}
inline ResourceDesc ResourceDesc::CubeMap(uint64_t width, uint32_t height, eFormat format,
										  eResourceFlags flags,
										  uint16_t mipLevels, uint32_t multisampleCount, uint32_t multisampleQuality,
										  uint64_t alignment, eTextureLayout layout) {
	ResourceDesc desc;
	desc.type = eResourceType::TEXTURE;

	desc.textureDesc.dimension = eTextueDimension::TWO;
	desc.textureDesc.format = format;
	desc.textureDesc.width = width;
	desc.textureDesc.height = height;
	desc.textureDesc.depthOrArraySize = 6;

	desc.textureDesc.flags = flags;
	desc.textureDesc.mipLevels = mipLevels;
	desc.textureDesc.multisampleCount = multisampleCount;
	desc.textureDesc.multisampleQuality = multisampleQuality;
	desc.textureDesc.alignment = alignment;
	desc.textureDesc.layout = layout;

	return desc;
}
inline ResourceDesc ResourceDesc::CubeMapArray(uint64_t width, uint32_t height, eFormat format, uint16_t arraySize,
											   eResourceFlags flags,
											   uint16_t mipLevels, uint32_t multisampleCount, uint32_t multisampleQuality,
											   uint64_t alignment, eTextureLayout layout) {
	assert(arraySize <= 65535 / 6);
	ResourceDesc desc;
	desc.type = eResourceType::TEXTURE;

	desc.textureDesc.dimension = eTextueDimension::TWO;
	desc.textureDesc.format = format;
	desc.textureDesc.width = width;
	desc.textureDesc.height = height;
	desc.textureDesc.depthOrArraySize = 6 * arraySize;

	desc.textureDesc.flags = flags;
	desc.textureDesc.mipLevels = mipLevels;
	desc.textureDesc.multisampleCount = multisampleCount;
	desc.textureDesc.multisampleQuality = multisampleQuality;
	desc.textureDesc.alignment = alignment;
	desc.textureDesc.layout = layout;

	return desc;
}


inline RootParameterDesc RootParameterDesc::Constant(unsigned numConstants, unsigned shaderRegister, unsigned registerSpace, eShaderVisiblity shaderVisibility) {
	RootParameterDesc desc;

	desc.m_type = RootParameterDesc::CONSTANT;
	desc.constant.numConstants = numConstants;
	desc.constant.shaderRegister = shaderRegister;
	desc.constant.registerSpace = registerSpace;
	desc.m_shaderVisibility = shaderVisibility;

	return desc;
}
inline RootParameterDesc RootParameterDesc::Cbv(unsigned shaderRegister, unsigned registerSpace, eShaderVisiblity shaderVisibility) {
	RootParameterDesc desc;

	desc.m_type = RootParameterDesc::CBV;
	desc.descriptor.shaderRegister = shaderRegister;
	desc.descriptor.registerSpace = registerSpace;
	desc.m_shaderVisibility = shaderVisibility;

	return desc;
}
inline RootParameterDesc RootParameterDesc::Srv(unsigned shaderRegister, unsigned registerSpace, eShaderVisiblity shaderVisibility) {
	RootParameterDesc desc;

	desc.m_type = RootParameterDesc::SRV;
	desc.descriptor.shaderRegister = shaderRegister;
	desc.descriptor.registerSpace = registerSpace;
	desc.m_shaderVisibility = shaderVisibility;

	return desc;
}
inline RootParameterDesc RootParameterDesc::Uav(unsigned shaderRegister, unsigned registerSpace, eShaderVisiblity shaderVisibility) {
	RootParameterDesc desc;

	desc.m_type = RootParameterDesc::UAV;
	desc.descriptor.shaderRegister = shaderRegister;
	desc.descriptor.registerSpace = registerSpace;
	desc.m_shaderVisibility = shaderVisibility;

	return desc;
}
inline RootParameterDesc RootParameterDesc::DescriptorTable(unsigned numDescriptorRanges, DescriptorRange* descriptorRanges, eShaderVisiblity shaderVisibility) {
	RootParameterDesc desc;

	desc.m_type = RootParameterDesc::DESCRIPTOR_TABLE;
	new (&desc.descriptorTable) RootDescriptorTable();
	desc.descriptorTable.ranges.assign(descriptorRanges, descriptorRanges + numDescriptorRanges);
	desc.m_shaderVisibility = shaderVisibility;

	return desc;
}
inline RootParameterDesc RootParameterDesc::DescriptorTable(std::vector<DescriptorRange> descriptorRanges, eShaderVisiblity shaderVisibility) {
	RootParameterDesc desc;

	desc.m_type = RootParameterDesc::DESCRIPTOR_TABLE;
	new (&desc.descriptorTable) RootDescriptorTable();
	desc.descriptorTable.ranges = std::move(descriptorRanges);
	desc.m_shaderVisibility = shaderVisibility;

	return desc;
}
inline RootParameterDesc RootParameterDesc::DescriptorTable(eShaderVisiblity shaderVisibility) {
	RootParameterDesc desc;

	desc.m_type = RootParameterDesc::DESCRIPTOR_TABLE;
	new (&desc.descriptorTable) RootDescriptorTable();
	desc.m_shaderVisibility = shaderVisibility;

	return desc;
}


template <>
inline const RootConstant& RootParameterDesc::As<RootParameterDesc::eType::CONSTANT>() const {
	if (m_type != eType::CONSTANT)
		throw InvalidCastException("Object has different type than requested.");
	return constant;
}

template <>
inline const RootDescriptor& RootParameterDesc::As<RootParameterDesc::eType::CBV>() const {
	if (m_type != eType::CBV)
		throw InvalidCastException("Object has different type than requested.");
	return descriptor;
}

template <>
inline const RootDescriptor& RootParameterDesc::As<RootParameterDesc::eType::SRV>() const {
	if (m_type != eType::SRV)
		throw InvalidCastException("Object has different type than requested.");
	return descriptor;
}

template <>
inline const RootDescriptor& RootParameterDesc::As<RootParameterDesc::eType::UAV>() const {
	if (m_type != eType::UAV)
		throw InvalidCastException("Object has different type than requested.");
	return descriptor;
}

template <>
inline const RootDescriptorTable& RootParameterDesc::As<RootParameterDesc::eType::DESCRIPTOR_TABLE>() const {
	if (m_type != eType::DESCRIPTOR_TABLE)
		throw InvalidCastException("Object has different type than requested.");
	return descriptorTable;
}

template <>
inline RootConstant& RootParameterDesc::As<RootParameterDesc::eType::CONSTANT>() {
	if (m_type != eType::CONSTANT)
		throw InvalidCastException("Object has different type than requested.");
	return constant;
}

template <>
inline RootDescriptor& RootParameterDesc::As<RootParameterDesc::eType::CBV>() {
	if (m_type != eType::CBV)
		throw InvalidCastException("Object has different type than requested.");
	return descriptor;
}

template <>
inline RootDescriptor& RootParameterDesc::As<RootParameterDesc::eType::SRV>() {
	if (m_type != eType::SRV)
		throw InvalidCastException("Object has different type than requested.");
	return descriptor;
}

template <>
inline RootDescriptor& RootParameterDesc::As<RootParameterDesc::eType::UAV>() {
	if (m_type != eType::UAV)
		throw InvalidCastException("Object has different type than requested.");
	return descriptor;
}

template <>
inline RootDescriptorTable& RootParameterDesc::As<RootParameterDesc::eType::DESCRIPTOR_TABLE>() {
	if (m_type != eType::DESCRIPTOR_TABLE)
		throw InvalidCastException("Object has different type than requested.");
	return descriptorTable;
}



//------------------------------------------------------------------------------
// Internal helper function
//------------------------------------------------------------------------------

inline unsigned GetFormatSizeInBytes(eFormat format) {
	switch (format) {
		case eFormat::R32G32B32A32_TYPELESS:
		case eFormat::R32G32B32A32_FLOAT:
		case eFormat::R32G32B32A32_UINT:
		case eFormat::R32G32B32A32_SINT:
			return 4 * 4;
		case eFormat::R32G32B32_TYPELESS:
		case eFormat::R32G32B32_FLOAT:
		case eFormat::R32G32B32_UINT:
		case eFormat::R32G32B32_SINT:
			return 3 * 4;
		case eFormat::R16G16B16A16_TYPELESS:
		case eFormat::R16G16B16A16_FLOAT:
		case eFormat::R16G16B16A16_UNORM:
		case eFormat::R16G16B16A16_UINT:
		case eFormat::R16G16B16A16_SNORM:
		case eFormat::R16G16B16A16_SINT:
			return 4 * 2;
		case eFormat::R32G32_TYPELESS:
		case eFormat::R32G32_FLOAT:
		case eFormat::R32G32_UINT:
		case eFormat::R32G32_SINT:
			return 2 * 4;
		case eFormat::R32G8X24_TYPELESS:
		case eFormat::D32_FLOAT_S8X24_UINT:
		case eFormat::R32_FLOAT_X8X24_TYPELESS:
		case eFormat::X32_TYPELESS_G8X24_UINT:
			return 8;
		case eFormat::R10G10B10A2_TYPELESS:
		case eFormat::R10G10B10A2_UNORM:
		case eFormat::R10G10B10A2_UINT:
		case eFormat::R11G11B10_FLOAT:
			return 4 * 1;
		case eFormat::R8G8B8A8_TYPELESS:
		case eFormat::R8G8B8A8_UNORM:
		case eFormat::R8G8B8A8_UNORM_SRGB:
		case eFormat::R8G8B8A8_UINT:
		case eFormat::R8G8B8A8_SNORM:
		case eFormat::R8G8B8A8_SINT:
			return 4 * 1;
		case eFormat::R16G16_TYPELESS:
		case eFormat::R16G16_FLOAT:
		case eFormat::R16G16_UNORM:
		case eFormat::R16G16_UINT:
		case eFormat::R16G16_SNORM:
		case eFormat::R16G16_SINT:
			return 2 * 2;
		case eFormat::R32_TYPELESS:
		case eFormat::D32_FLOAT:
		case eFormat::R32_FLOAT:
		case eFormat::R32_UINT:
		case eFormat::R32_SINT:
			return 1 * 4;
		case eFormat::R24G8_TYPELESS:
		case eFormat::D24_UNORM_S8_UINT:
		case eFormat::R24_UNORM_X8_TYPELESS:
		case eFormat::X24_TYPELESS_G8_UINT:
			return 4;
		case eFormat::R8G8_TYPELESS:
		case eFormat::R8G8_UNORM:
		case eFormat::R8G8_UINT:
		case eFormat::R8G8_SNORM:
		case eFormat::R8G8_SINT:
			return 2 * 1;
		case eFormat::R16_TYPELESS:
		case eFormat::R16_FLOAT:
		case eFormat::D16_UNORM:
		case eFormat::R16_UNORM:
		case eFormat::R16_UINT:
		case eFormat::R16_SNORM:
		case eFormat::R16_SINT:
			return 1 * 2;
		case eFormat::R8_TYPELESS:
		case eFormat::R8_UNORM:
		case eFormat::R8_UINT:
		case eFormat::R8_SNORM:
		case eFormat::R8_SINT:
		case eFormat::A8_UNORM:
			return 1 * 1;
		default:
			return 0;
	}
}


} // namespace inl::gxapi
