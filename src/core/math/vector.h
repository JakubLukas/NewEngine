#pragma once


namespace Veng
{


struct Vector2
{
	static const Vector2 AXIS_X;
	static const Vector2 AXIS_Y;

	Vector2();
	Vector2(float x, float y);
	Vector2(const Vector2& vec);
	Vector2& operator=(const Vector2& other);

	Vector2 operator+() const;
	Vector2 operator-() const;

	Vector2 operator+(const Vector2& other) const;
	Vector2 operator-(const Vector2& other) const;

	float x;
	float y;
};


struct Vector3
{
	static const Vector3 AXIS_X;
	static const Vector3 AXIS_Y;
	static const Vector3 AXIS_Z;

	Vector3();
	Vector3(float x, float y, float z);
	Vector3(const Vector2& vec2, float z);
	Vector3(const Vector3& vec);
	Vector3& operator=(const Vector3& vec);

	Vector3 operator+() const;
	Vector3 operator-() const;

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
	static const Vector4 AXIS_X;
	static const Vector4 AXIS_Y;
	static const Vector4 AXIS_Z;

	Vector4();
	Vector4(float x, float y, float z, float w);
	Vector4(const Vector3& vec3, float w);
	Vector4(const Vector4& vec);
	Vector4& operator=(const Vector4& vec);

	Vector4 operator+(const Vector4& other) const;
	Vector4 operator-(const Vector4& other) const;

	float Length() const;
	void Normalize();

	Vector3 GetXYZ() const;
	Vector3 GetVector3() const;

	static float Dot(const Vector4& vec1, const Vector4& vec2);


	float x;
	float y;
	float z;
	float w;
};


}