#pragma once

#include <BaseLibrary/Color.hpp>
#include <GraphicsEngine/Resources/IFont.hpp>


namespace inl::gui {


struct ControlStyle {
	static ControlStyle Dark(ColorF accent)	{
		ControlStyle style;
		style.background = { 0.16f, 0.16f, 0.16f, 1 };
		style.foreground = { 0.24f, 0.24f, 0.24f, 1 };
		style.hover.v.xyz = (Vec3)style.background.v.xyz + 0.2f * (Vec3)accent.v.xyz;
		style.focus = { 0.32f, 0.32f, 0.32f, 1 };
		style.pressed = { 0.10f, 0.10f, 0.10f, 1 };
		style.accent = accent;

		style.text = { 0.8f, 0.8f, 0.8f, 1 };
		style.selection = { 0.2f, 0.3f, 0.8f, 1.0f };

		return style;
	}

	ColorF background = { 0.16f, 0.16f, 0.16f, 1 };
	ColorF foreground = { 0.24f, 0.24f, 0.24f, 1 };
	ColorF hover = { 0.24f, 0.32f, 0.30f, 1 };
	ColorF focus = { 0.32f, 0.32f, 0.32f, 1 };
	ColorF pressed = { 0.10f, 0.10f, 0.10f, 1 };
	ColorF accent = { 0.24f, 0.45f, 0.37f, 1 };

	ColorF text = { 0.8f, 0.8f, 0.8f, 1 };
	ColorF selection = { 0.2f, 0.3f, 0.8f, 1.0f };

	gxeng::IFont* font = nullptr;
	float fontSize = 12.0f;
};


} // namespace inl::gui