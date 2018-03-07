#pragma once

#include <InlineMath.hpp>

#include <string>

namespace inl::gxeng {


/// <summary>
/// An orthographic camera for top-viewing 2D scenes.
/// </summary>
class Camera2D {
public:
	Camera2D();
	virtual ~Camera2D() {} // This is virtual solely for the ObservableCamera2D helper in GraphicsEngine.

	/// <summary> You can identify cameras by their name in the pipeline graph. </summary>
	void SetName(std::string name);

	/// <summary> You can identify cameras by their name in the pipeline graph. Set it first to your taste. </summary>
	const std::string& GetName() const;


	/// <summary> This is going to be shown on the center of the rendered image. </summary>
	void SetPosition(Vec2 position);

	/// <summary> Rotate the camera (not the scene) anti-clockwise (as in math). </summary>
	/// <param name="rotation"> In radians, CCW. </param>
	void SetRotation(float rotation);


	/// <summary> Get which world-space point is the center of the rendered image. </summary>
	Vec2 GetPosition() const;

	/// <summary> Get how many radians the camera is rotated CCW. </summary>
	float GetRotation() const;

	/// <summary> The size of the rectangle the camera's field of view covers. </summary>
	void SetExtent(Vec2 extent);

	/// <summary> The size of the rectangle the camera's field of view covers. </summary>
	/// <remarks> The rectangle's center point corresponds to <see cref="SetPosition/>. </remarks>
	Vec2 GetExtent() const;

	/// <summary> The matrix transforms world space to camera space, with Y being screen-up post-transform. </summary>
	Mat33 GetViewMatrix() const;

	/// <summary> The matrix takes the rectangle defined by <see cref="SetExtent"/> to the unit rectangle [-1,1]x[-1,1]. </summary>
	Mat33 GetProjectionMatrix() const;
private:
	std::string m_name;
	Vec2 m_position;
	float m_rotation;
	Vec2 m_extent;
};


} // namespace inl::gxeng
