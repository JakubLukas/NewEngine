#pragma once


struct Vector3
{
	explicit Vector3(float x, float y, float z);
	explicit Vector3(const Vector3& vec);

	float x;
	float y;
	float z;
};