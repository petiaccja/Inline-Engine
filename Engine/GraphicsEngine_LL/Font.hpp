#pragma once

#include <GraphicsEngine/Resources/IFont.hpp>

#include <string>
#include <unordered_map>

#include "Image.hpp"


namespace inl::gxeng {



class Font : public IFont {
public:
	struct GlyphInfo {
		float advance;
		Vec2i atlasPos;
		Vec2i atlasSize;
	};
public:
	Font(Image atlas);

	/// <summary> Loads an already opened TTF font file. </summary>
	void LoadFile(std::istream& file) override;

	/// <summary> Load TTF file from memory. </summary>
	void LoadFile(const void* data, size_t size) override;

	/// <summary> Check weather given character can be rendered. </summary>
	/// <param name="character"> UCS-4 code point. </param>
	bool IsCharacterSupported(char32_t character) const override;

	/// <summary> Returns the width of the specified character. </summary>
	/// <param name="character"> UCS-4 code point. </param>
	/// <exception cref="OutOfRangeException"> If character cannot be rendered. </exception>
	GlyphInfo GetGlyphInfo(char32_t character) const;

	/// <summary> Returns the texture atlas that contain the rasterized letters. </summary>
	const Image& GetGlyphAtlas() const;

	/// <summary> Calculates the height of a line of text for given font size. </summary>
	/// <param name="fontSize"> The size of the font you want to render. </param>
	float CalculateTextHeight(float fontSize) const override;
private:
	Image m_atlas;
	std::unordered_map<char32_t, GlyphInfo> m_glyphs;
};



} // namespace inl::gxeng