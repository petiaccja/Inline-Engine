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

	void LoadFile(std::istream& file) override;
	void LoadFile(const void* data, size_t size) override;
	bool IsCharacterSupported(char32_t character) const override;
	float CalculateTextHeight(float fontSize) const override;
	float CalculateTextWidth(std::u32string_view text, float fontSize) const override;
	intptr_t FindCharacter(std::u32string_view text, float coordinate, float fontSize) const override;
	std::pair<float, float> FindCoordinates(std::u32string_view text, size_t index, float fontSize) const override;

	/// <summary> Returns the width of the specified character. </summary>
	/// <param name="character"> UCS-4 code point. </param>
	/// <exception cref="OutOfRangeException"> If character cannot be rendered. </exception>
	GlyphInfo GetGlyphInfo(char32_t character) const;

	/// <summary> Returns the texture atlas that contain the rasterized letters. </summary>
	const Image& GetGlyphAtlas() const;
private:
	Image m_atlas;
	std::unordered_map<char32_t, GlyphInfo> m_glyphs;
};



} // namespace inl::gxeng