#include "camera.h"

#include "core/math/math.h"


namespace Veng
{


Ray ray_from_camera_center(const Transform& camTransform, const Camera& camera)
{
	return { camTransform.position, Quaternion::Multiply(camTransform.rotation, Vector3::AXIS_Z) };
}

Ray ray_from_screen_coords(const Transform& camTransform, const Camera& camera, const Vector2& screenCoords)
{
	Ray ray;
	ray.origin = camTransform.position;
	//compute correct ray
	float mx = (screenCoords.x - camera.screenWidth * 0.5f) * (1.0f / camera.screenWidth) * 2.0f;
	float my = -(screenCoords.y - camera.screenHeight * 0.5f) * (1.0f / camera.screenHeight) * 2.0f;

	Vector3 camRight = Quaternion::Multiply(camTransform.rotation, Vector3::AXIS_X);
	Vector3 camUp = Quaternion::Multiply(camTransform.rotation, Vector3::AXIS_Y);
	Vector3 camDir = Quaternion::Multiply(camTransform.rotation, Vector3::AXIS_Z);

	float fov_tan = tanf(camera.fov * 0.5f);

	Vector3 screenCenter = camTransform.position + camDir * camera.nearPlane;
	Vector3 screenRightDir = camRight * fov_tan * camera.nearPlane * camera.aspect;
	Vector3 screenUpDir = camUp * fov_tan * camera.nearPlane;
	Vector3 screenPoint = screenCenter + screenRightDir * mx + screenUpDir * my;
	ray.direction = screenPoint - camTransform.position;
	ray.direction.Normalize();

	return ray;
}


}