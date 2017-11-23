#pragma once


namespace Veng
{


struct Vector3
{
	Vector3();
	Vector3(float x, float y, float z);
	Vector3(const Vector3& vec);

	Vector3& operator=(const Vector3& vec);

	Vector3 operator+(const Vector3& other) const;
	Vector3 operator-(const Vector3& other) const;

	float Length() const;
	void Normalize();

	static float Dot(const Vector3& vec1, const Vector3& vec2);
	static Vector3 Cross(const Vector3& vec1, const Vector3& vec2); //left hand


	float x;
	float y;
	float z;
};


struct Vector4
{
	Vector4();
	Vector4(float x, float y, float z, float w);
	Vector4(const Vector3& vec3, float w);
	Vector4(const Vector4& vec);

	Vector4& operator=(const Vector4& vec);


	float x;
	float y;
	float z;
	float w;
};


}