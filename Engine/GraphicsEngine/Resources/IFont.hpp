#pragma once


#include <istream>



namespace inl::gxeng {


class IFont {
public:
	virtual ~IFont() = default;

	/// <summary> Loads an already opened TTF font file. </summary>
	virtual void LoadFile(std::istream& file) = 0;

	/// <summary> Load TTF file from memory. </summary>
	virtual void LoadFile(const void* data, size_t size) = 0;

	/// <summary> Check weather given character can be rendered. </summary>
	/// <param name="character"> UCS-4 code point. </param>
	virtual bool IsCharacterSupported(char32_t character) const = 0;

	/// <summary> Calculates the height of a line of text for given font size. </summary>
	/// <param name="fontSize"> The size of the font you want to render. </param>
	virtual float CalculateTextHeight(float fontSize) const = 0;

	/// <summary> Calculates the length of a given string at a given font size. </summary>
	/// <returns> Total width of <paramref name="text">, same unit as <paramref name="fontSize"/> </returns>
	virtual float CalculateTextWidth(std::u32string_view text, float fontSize) const = 0;

	/// <summary> Get the index of the character at specified coordinate. </summary>
	/// <param name="text"> Get index in this string. </param>
	/// <param name="coordinate"> Same units as fontSize. 0 is the left side of text. </param>
	/// <param name="fontSize"> Size of the font to use for the search. Arbitrary units. </param>
	/// <returns> -1 if coordinate is at left of text (i.e. is negative), text.size() if at right of text,
	///		and the actual index of the character under <paramref name="coordinate"/> otherwise. </returns>
	virtual intptr_t FindCharacter(std::u32string_view text, float coordinate, float fontSize) const = 0;

	/// <summary> Get the coordinates of a character at a specific index. </summary>
	/// <param name="text"> Get coordinate of character in this string. </param>
	/// <param name="index"> Which character to get coordinates of. </param>
	/// <param name="fontSize"> Size of the font to use for the search. Arbitrary units. </param>
	/// <returns> A pair to the coordinate interval the character spans. Same unit as fontSize. </param>
	virtual std::pair<float, float> FindCoordinates(std::u32string_view text, size_t index, float fontSize) const = 0;
};





} // namespace inl::gxeng