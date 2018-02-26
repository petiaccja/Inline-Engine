#include "Font.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace inl::gxeng {

Font::Font(Image atlas)
	: m_atlas(std::move(atlas))
{

}


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


void Font::SetFamily(std::string familyName, bool bold, bool italic) {
	FT_Error error;

	FT_Library library;
	FT_Face face;

	// Init freetype.
	error = FT_Init_FreeType(&library);

	// Load font.
	error = FT_New_Face(library, R"(C:\Windows\Fonts\arial.ttf)", 0, &face);
	error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);

	// Set font properties.
	int sizeInPixels = 16;
	error = FT_Set_Pixel_Sizes(face, 0, sizeInPixels);

	AtlasHelper atlasPixels;
	int totalWidth = 0;
	int maxHeight = 0;

	// Render 0-255 characters.
	for (char32_t ch = 0; ch < 256; ++ch) {
		unsigned glyphIndex = FT_Get_Char_Index(face, ch);
		if (glyphIndex == 0) {
			continue;
		}

		error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		Vec2i position = { totalWidth, 0 };
		Vec2i size = { 
			face->glyph->bitmap_left + face->glyph->bitmap.width,
			face->glyph->bitmap_top + face->glyph->bitmap.rows
		};
		totalWidth += size.x;
		maxHeight = std::max(size.y, maxHeight);

		m_glyphs[ch] = GlyphInfo{
			float(face->glyph->advance.x/64),
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
}


bool Font::SupportsCharacter(char32_t character) const {
	auto it = m_glyphs.find(character);
	return it != m_glyphs.end();
}


Font::GlyphInfo Font::GetCharacterInfo(char32_t character) const {
	auto it = m_glyphs.find(character);
	if (it == m_glyphs.end()) {
		throw OutOfRangeException("Character cannot be rendered.");
	}

	return it->second;
}


} // namespace inl::gxeng