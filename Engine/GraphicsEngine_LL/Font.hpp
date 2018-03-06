#pragma once

#include <string>
#include <unordered_map>

#include "Image.hpp"


namespace inl::gxeng {



class Font {
public:
	struct GlyphInfo {
		float advance;
		Vec2i atlasPos;
		Vec2i atlasSize;
	};
public:
	Font(Image atlas);

	void SetFamily(std::string familyName, bool bold, bool italic);
	void SetFile(std::istream& file);
	void SetFile(const void* data, size_t size);

	/// <summary> Check weather given character can be rendered. </summary>
	/// <param name="character"> UCS-4 code point. </param>
	bool SupportsCharacter(char32_t character) const;

	/// <summary> Returns the width of the specified character. </summary>
	/// <param name="character"> UCS-4 code point. </param>
	/// <exception cref="OutOfRangeException"> If character cannot be rendered. </exception>
	GlyphInfo GetCharacterInfo(char32_t character) const;

	const Image* GetAtlas() const;
private:
	Image m_atlas;
	std::unordered_map<char32_t, GlyphInfo> m_glyphs;
};



} // namespace inl::gxeng