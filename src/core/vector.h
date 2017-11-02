#pragma once


namespace Veng
{


struct Vector3
{
	Vector3();
	Vector3(float x, float y, float z);
	explicit Vector3(const Vector3& vec);

	Vector3& operator =(const Vector3& other);

	float x;
	float y;
	float z;
};


}