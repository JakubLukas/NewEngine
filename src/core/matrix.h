#pragma once

#include "vector.h"


namespace Veng
{

//right hand coordinate system (z from display (DirectX style))

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


	//lines * columns
	static Matrix44 Multiply(const Matrix44& mat1, const Matrix44& mat2);

	//lines * vector
	static Vector4 Multiply(const Matrix44& mat, const Vector4& vec);

	// vector * columns
	static Vector4 Multiply(const Vector4& vec, const Matrix44& mat);

	void SetPerspective(float fovY, float ratio, float near, float far, bool homogenDepth);

	void SetRotateX(float angle);
	void SetRotateY(float angle);
	void SetRotateZ(float angle);

	void RotateX(float angle);
	void RotateY(float angle);
	void RotateZ(float angle);


	float m11, m12, m13, m14;
	float m21, m22, m23, m24;
	float m31, m32, m33, m34;
	float m41, m42, m43, m44;
};


Matrix44 operator*(const Matrix44& mat1, const Matrix44& mat2);
Vector4 operator*(const Matrix44& mat, const Vector4& vec);
Vector4 operator*(const Vector4& vec, const Matrix44& mat);


}