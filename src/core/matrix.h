#pragma once

#include "vector.h"
#include "quaternion.h"


namespace Veng
{


struct Matrix44
{
	static const Matrix44 IDENTITY;

	Matrix44();
	Matrix44(
		float r11, float r12, float r13, float r14,
		float r21, float r22, float r23, float r24,
		float r31, float r32, float r33, float r34,
		float r41, float r42, float r43, float r44
	);


	static Matrix44 Multiply(const Matrix44& mat1, const Matrix44& mat2);

	static Vector4 Multiply(const Matrix44& mat, const Vector4& vec);

	static Vector4 Multiply(const Vector4& vec, const Matrix44& mat);

	//(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc)
	void SetPerspective(float fovY, float ratio, float near, float far, bool homogenDepth);


	float m11, m12, m13, m14;
	float m21, m22, m23, m24;
	float m31, m32, m33, m34;
	float m41, m42, m43, m44;
};


struct Transform
{
	Transform() {}
	Transform(const Quaternion& rot, const Vector3& pos)
	: rotation(rot), position(pos) {}
	Transform(const Transform& other)
		: rotation(other.rotation), position(other.position){}

	Quaternion rotation;
	Vector3 position;
};


}