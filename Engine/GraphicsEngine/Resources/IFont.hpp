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
};





} // namespace inl::gxeng