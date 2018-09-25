#pragma once

#include <InlineMath.hpp>
#include <string>


namespace inl::gxeng {

class ICamera2D {
public:
	virtual ~ICamera2D() = default;

	/// <summary> You can identify cameras by their name in the pipeline graph. </summary>
	virtual void SetName(std::string name) = 0;

	/// <summary> You can identify cameras by their name in the pipeline graph. Set it first to your taste. </summary>
	virtual const std::string& GetName() const = 0;


	/// <summary> This is going to be shown on the center of the rendered image. </summary>
	virtual void SetPosition(Vec2 position) = 0;

	/// <summary> Rotate the camera (not the scene) anti-clockwise (as in math). </summary>
	/// <param name="rotation"> In radians, CCW. </param>
	virtual void SetRotation(float rotation) = 0;

	/// <summary> If true, Y axis is flipped so that coordinates increase downwards. </summary>
	virtual void SetVerticalFlip(bool enable) = 0;


	/// <summary> Get which world-space point is the center of the rendered image. </summary>
	virtual Vec2 GetPosition() const = 0;

	/// <summary> Get how many radians the camera is rotated CCW. </summary>
	virtual float GetRotation() const = 0;

	/// <summary> The size of the rectangle the camera's field of view covers. </summary>
	virtual void SetExtent(Vec2 extent) = 0;

	/// <summary> The size of the rectangle the camera's field of view covers. </summary>
	/// <remarks> The rectangle's center point corresponds to <see cref="SetPosition/>. </remarks>
	virtual Vec2 GetExtent() const = 0;

	/// <summary> False if Y coordinates increase upwards. </summary>
	virtual bool GetVerticalFlip() const = 0;

	/// <summary> The matrix transforms world space to camera space, with Y being screen-up post-transform. </summary>
	virtual Mat33 GetViewMatrix() const = 0;

	/// <summary> The matrix takes the rectangle defined by <see cref="SetExtent"/> to the unit rectangle [-1,1]x[-1,1]. </summary>
	virtual Mat33 GetProjectionMatrix() const = 0;
};


} // namespace inl::gxeng