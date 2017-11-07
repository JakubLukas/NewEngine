#pragma once


namespace Veng
{


struct Vector3
{
	Vector3();
	Vector3(float x, float y, float z);
	explicit Vector3(const Vector3& vec);

<<<<<<< HEAD
	Vector3& operator=(const Vector3& vec);

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
=======
	Vector3& operator =(const Vector3& other);
>>>>>>> 453ce2f7f17ead375d14a42e856ce0145e48c646

	float x;
	float y;
	float z;
	float w;
};


}