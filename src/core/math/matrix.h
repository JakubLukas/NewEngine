#pragma once

#include "core/core.h"
#include "vector.h"
#include "quaternion.h"


namespace Veng
{

//right hand coordinate system (z from display (OpenGL style))

struct FORCE_ALIGNMENT(16) Matrix44
{
	static const Matrix44 IDENTITY;

	Matrix44();
	Matrix44(
		float r11, float r12, float r13, float r14,
		float r21, float r22, float r23, float r24,
		float r31, float r32, float r33, float r34,
		float r41, float r42, float r43, float r44
	);
	Matrix44(const Matrix44& other);

	Matrix44& operator=(const Matrix44& other);


	//lines * columns
	static Matrix44 Multiply(const Matrix44& mat1, const Matrix44& mat2);

	//lines * vector
	static Vector4 Multiply(const Matrix44& mat, const Vector4& vec);

	// vector * columns
	static Vector4 Multiply(const Vector4& vec, const Matrix44& mat);

	void SetOrthogonal(float left, float right, float bottom, float top, float near, float far, float offset, bool homogenDepth);

	void SetPerspective(float fovY, float ratio, float near, float far, bool homogenDepth);

	void SetLookAt(const Vector4& eye, const Vector4& at, const Vector4& up);

	void SetRotateX(float angle);
	void SetRotateY(float angle);
	void SetRotateZ(float angle);

	void SetRotation(const Quaternion& quat);

	void RotateX(float angle);
	void RotateY(float angle);
	void RotateZ(float angle);

	void SetTranslation(const Vector3& trans);

	void Transpose();


	float m11, m12, m13, m14;
	float m21, m22, m23, m24;
	float m31, m32, m33, m34;
	float m41, m42, m43, m44;
};

Matrix44 operator*(const Matrix44& mat1, const Matrix44& mat2);
Vector4 operator*(const Matrix44& mat, const Vector4& vec);
Vector4 operator*(const Vector4& vec, const Matrix44& mat);


struct Transform
{
	Transform();
	Transform(const Quaternion& rot, const Vector3& pos);
	Transform(const Transform& other);

	Matrix44 ToMatrix44() const;


	Quaternion rotation = Quaternion::IDENTITY;
	Vector3 position;
};


}