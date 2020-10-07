#pragma once

#include "core/int.h"
#include "core/ray.h"
#include "core/math/matrix.h"


namespace Veng
{


struct Camera
{
	enum class Type : u8
	{
		Orthogonal,
		Perspective
	};
	Type type;
	//3B padding
	float screenWidth;
	float screenHeight;
	float nearPlane;
	float farPlane;
	float fov;
	float aspect;
};


Ray ray_from_camera_center(const Transform& camTransform, const Camera& camera);

Ray ray_from_screen_coords(const Transform& camTransform, const Camera& camera, const Vector2& screenCoords);


}