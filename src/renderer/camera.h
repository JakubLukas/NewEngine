#pragma once

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
	float screenWidth;
	float screenHeight;
	float nearPlane;
	float farPlane;
	float fov;
	float aspect;
};


}