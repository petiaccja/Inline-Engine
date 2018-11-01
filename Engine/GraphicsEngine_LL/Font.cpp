#include "Font.hpp"
#include <BaseLibrary/Exception/Exception.hpp>
#include <BaseLibrary/Singleton.hpp>
#include <BaseLibrary/Range.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace inl::gxeng {

//------------------------------------------------------------------------------
// Helper classes.
//------------------------------------------------------------------------------

// Currently the texture atlas contains rasterized letters of only this size.
static constexpr int ATLAS_FONT_SIZE = 24;


// A small helper class for a resizeable 2D texture so that it's easy to append new
// rendered glyphs to the atlas.
class AtlasHelper {
public:
	AtlasHelper(int width = 0, int height = 0) : m_width(width), m_height(height), m_pixels(width*height) {}

	void Resize(int width, int height) {
		AtlasHelper resized(width, height);

		int overlapWidth = std::min(width, m_width);
		int overlapHeight = std::min(height, m_height);

		for (int y = 0; y<overlapHeight; ++y) {
			for (int x = 0; x<overlapWidth; ++x) {
				resized(x, y) = (*this)(x, y);
			}
		}

		*this = std::move(resized);
	}

	uint8_t& operator()(int x, int y) {
		return m_pixels[y*m_width + x];
	}
	uint8_t operator()(int x, int y) const {
		return m_pixels[y*m_width + x];
	}
	const uint8_t* Data() const { return m_pixels.data(); }
	int Width() const { return m_width; }
	int Height() const { return m_height; }
private:
	int m_width, m_height;
	std::vector<uint8_t> m_pixels;
};


// Throw an exception in a freetype function failed.
template <class ExceptionT, class... Args>
void ThrowIfFailed(FT_Error error, Args&&... args) {
	if (error != FT_Err_Ok) {
		throw ExceptionT(std::forward<Args>(args)...);
	}
}


// Initializes freetype lazily
class FreetypeInit {
public:
	FreetypeInit() {
		ThrowIfFailed<RuntimeException>(FT_Init_FreeType(&m_library), "Failed to initialize freetype.");
	}
	FT_Library GetFreetype() const {
		return m_library;
	}
private:
	FT_Library m_library;
};

using Freetype = Singleton<FreetypeInit>;



//------------------------------------------------------------------------------
// Font class.
//------------------------------------------------------------------------------


Font::Font(Image atlas)
	: m_atlas(std::move(atlas))
{}


void Font::LoadFile(std::istream& file) {
	std::vector<char> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	LoadFile(content.data(), content.size());
}


void Font::LoadFile(const void* data, size_t size) {
	FT_Library library = Freetype::GetInstance().GetFreetype();
	FT_Face face;

	// Load font.
	ThrowIfFailed<InvalidArgumentException>(FT_New_Memory_Face(library, (FT_Byte*)data, (FT_Long)size, 0, &face), "Failed to read font file.");
	ThrowIfFailed<InvalidArgumentException>(FT_Select_Charmap(face, FT_ENCODING_UNICODE), "Font does not have UNICODE glyph map.");

	// Set font properties.
	int sizeInPixels = ATLAS_FONT_SIZE;
	ThrowIfFailed<RuntimeException>(FT_Set_Pixel_Sizes(face, 0, sizeInPixels), "Could not set pixel size with freetype.");

	AtlasHelper atlasPixels;
	int totalWidth = 0;
	int maxHeight = 0;

	// Render 0-255 characters.
	for (char32_t ch = 0; ch < 256; ++ch) {
		unsigned glyphIndex = FT_Get_Char_Index(face, ch);
		if (glyphIndex == 0) {
			continue;
		}

		ThrowIfFailed<RuntimeException>(FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT), "Freetype could not load glyph.", std::to_string(glyphIndex));
		ThrowIfFailed<RuntimeException>(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL), "Freetype could not render glyph.", std::to_string(glyphIndex));

		Vec2i position = { totalWidth, 0 };
		Vec2i size = { 
			face->glyph->bitmap_left + face->glyph->bitmap.width,
			face->glyph->bitmap_top + face->glyph->bitmap.rows
		};
		totalWidth += size.x + 2;
		maxHeight = std::max(size.y, maxHeight);

		m_glyphs[ch] = GlyphInfo{
			float(face->glyph->advance.x)/64.f/sizeInPixels,
			position,
			size
		};

		// Copy rendered glyph to texture atlas.
		if (maxHeight > atlasPixels.Height() || totalWidth > atlasPixels.Width()) {
			atlasPixels.Resize(totalWidth, maxHeight);
		}
		for (unsigned y = 0; y<face->glyph->bitmap.rows; ++y) {
			for (unsigned x = 0; x<face->glyph->bitmap.width; ++x) {
				Vec2i pxPos = position + Vec2i(x, y) + Vec2i(face->glyph->bitmap_left, sizeInPixels-face->glyph->bitmap_top);
				atlasPixels(pxPos.x, pxPos.y) = face->glyph->bitmap.buffer[y*face->glyph->bitmap.pitch + x];
			}
		}
	}

	// Upload atlas to GPU.
	m_atlas.SetLayout(atlasPixels.Width(), atlasPixels.Height(), ePixelChannelType::INT8_NORM, 1, ePixelClass::LINEAR);
	const IPixelReader& pixelReader = Pixel<ePixelChannelType::INT8_NORM, 1, ePixelClass::LINEAR>::Reader();
	m_atlas.Update(0, 0, atlasPixels.Width(), atlasPixels.Height(), 0, atlasPixels.Data(), pixelReader);
	const_cast<Texture2D&>(m_atlas.GetSrv().GetResource()).SetName("font atlas");
}


bool Font::IsCharacterSupported(char32_t character) const {
	auto it = m_glyphs.find(character);
	return it != m_glyphs.end();
}


Font::GlyphInfo Font::GetGlyphInfo(char32_t character) const {
	auto it = m_glyphs.find(character);
	if (it == m_glyphs.end()) {
		throw OutOfRangeException("Character cannot be rendered.");
	}

	return it->second;
}


const Image& Font::GetGlyphAtlas() const {
	return m_atlas;
}


float Font::CalculateTextHeight(float fontSize) const {
	assert(fontSize > 0);
	return m_atlas.GetHeight() * fontSize / ATLAS_FONT_SIZE;
}


float Font::CalculateTextWidth(std::u32string_view text, float fontSize) const {
	assert(fontSize > 0);
	float width = 0.0f;
	for (auto& glyph : text) {
		if (IsCharacterSupported(glyph)) {
			const auto& info = GetGlyphInfo(glyph);
			width += info.advance;
		}
	}
	return width * fontSize;
}


intptr_t Font::FindCharacter(std::u32string_view text, float coordinate, float fontSize) const {
	assert(fontSize > 0);
	if (coordinate < 0) {
		return -1;
	}
	float width = 0.0f;
	for (auto glyphIdx : Range(intptr_t(text.size()))) {
		const auto& info = GetGlyphInfo(text[glyphIdx]);
		width += info.advance*fontSize;
		if (width >= coordinate) {
			return glyphIdx;
		}
	}
	return (intptr_t)text.size();
}


std::pair<float, float> Font::FindCoordinates(std::u32string_view text, size_t index, float fontSize) const {
	assert(index < text.size());
	assert(fontSize > 0);
	float precedingWidth = CalculateTextWidth(text.substr(0, index), fontSize);
	float glyphWidth = GetGlyphInfo(text[index]).advance*fontSize;
	return { precedingWidth, precedingWidth + glyphWidth };
}


} // namespace inl::gxeng