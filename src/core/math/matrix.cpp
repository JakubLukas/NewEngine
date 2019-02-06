#include "matrix.h"

#include "core/memory.h"
#include "math.h"


namespace Veng
{


const Matrix44 Matrix44::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);


Matrix44::Matrix44()
	: m11(0.0f), m12(0.0f), m13(0.0f), m14(0.0f)
	, m21(0.0f), m22(0.0f), m23(0.0f), m24(0.0f)
	, m31(0.0f), m32(0.0f), m33(0.0f), m34(0.0f)
	, m41(0.0f), m42(0.0f), m43(0.0f), m44(0.0f)
{

}

Matrix44::Matrix44(
	float m11, float m12, float m13, float m14,
	float m21, float m22, float m23, float m24,
	float m31, float m32, float m33, float m34,
	float m41, float m42, float m43, float m44
)
	: m11(m11), m12(m12), m13(m13), m14(m14)
	, m21(m21), m22(m22), m23(m23), m24(m24)
	, m31(m31), m32(m32), m33(m33), m34(m34)
	, m41(m41), m42(m42), m43(m43), m44(m44)
{
}

Matrix44::Matrix44(const Matrix44& other)
{
	memory::Copy(this, &other, sizeof(float) * 16);
}

Matrix44& Matrix44::operator=(const Matrix44& other)
{
	memory::Copy(this, &other, sizeof(float) * 16);
	return *this;
}


Matrix44 Matrix44::Multiply(const Matrix44& mat1, const Matrix44& mat2)
{
	return Matrix44
	(
		mat1.m11 * mat2.m11 + mat1.m12 * mat2.m21 + mat1.m13 * mat2.m31 + mat1.m14 * mat2.m41,
		mat1.m11 * mat2.m12 + mat1.m12 * mat2.m22 + mat1.m13 * mat2.m32 + mat1.m14 * mat2.m42,
		mat1.m11 * mat2.m13 + mat1.m12 * mat2.m23 + mat1.m13 * mat2.m33 + mat1.m14 * mat2.m43,
		mat1.m11 * mat2.m14 + mat1.m12 * mat2.m24 + mat1.m13 * mat2.m34 + mat1.m14 * mat2.m44,

		mat1.m21 * mat2.m11 + mat1.m22 * mat2.m21 + mat1.m23 * mat2.m31 + mat1.m24 * mat2.m41,
		mat1.m21 * mat2.m12 + mat1.m22 * mat2.m22 + mat1.m23 * mat2.m32 + mat1.m24 * mat2.m42,
		mat1.m21 * mat2.m13 + mat1.m22 * mat2.m23 + mat1.m23 * mat2.m33 + mat1.m24 * mat2.m43,
		mat1.m21 * mat2.m14 + mat1.m22 * mat2.m24 + mat1.m23 * mat2.m34 + mat1.m24 * mat2.m44,

		mat1.m31 * mat2.m11 + mat1.m32 * mat2.m21 + mat1.m33 * mat2.m31 + mat1.m34 * mat2.m41,
		mat1.m31 * mat2.m12 + mat1.m32 * mat2.m22 + mat1.m33 * mat2.m32 + mat1.m34 * mat2.m42,
		mat1.m31 * mat2.m13 + mat1.m32 * mat2.m23 + mat1.m33 * mat2.m33 + mat1.m34 * mat2.m43,
		mat1.m31 * mat2.m14 + mat1.m32 * mat2.m24 + mat1.m33 * mat2.m34 + mat1.m34 * mat2.m44,

		mat1.m41 * mat2.m11 + mat1.m42 * mat2.m21 + mat1.m43 * mat2.m31 + mat1.m44 * mat2.m41,
		mat1.m41 * mat2.m12 + mat1.m42 * mat2.m22 + mat1.m43 * mat2.m32 + mat1.m44 * mat2.m42,
		mat1.m41 * mat2.m13 + mat1.m42 * mat2.m23 + mat1.m43 * mat2.m33 + mat1.m44 * mat2.m43,
		mat1.m41 * mat2.m14 + mat1.m42 * mat2.m24 + mat1.m43 * mat2.m34 + mat1.m44 * mat2.m44
	);
}


Vector4 Matrix44::Multiply(const Matrix44& mat, const Vector4& vec)
{
	return Vector4(
		mat.m11 * vec.x + mat.m12 * vec.y + mat.m13 * vec.z + mat.m14 * vec.w,
		mat.m21 * vec.x + mat.m22 * vec.y + mat.m23 * vec.z + mat.m24 * vec.w,
		mat.m31 * vec.x + mat.m32 * vec.y + mat.m33 * vec.z + mat.m34 * vec.w,
		mat.m41 * vec.x + mat.m42 * vec.y + mat.m43 * vec.z + mat.m44 * vec.w
	);
}


Vector4 Matrix44::Multiply(const Vector4& vec, const Matrix44& mat)
{
	return Vector4(
		vec.x * mat.m11 + vec.y * mat.m21 + vec.z * mat.m31 + vec.w * mat.m41,
		vec.x * mat.m12 + vec.y * mat.m22 + vec.z * mat.m32 + vec.w * mat.m42,
		vec.x * mat.m13 + vec.y * mat.m23 + vec.z * mat.m33 + vec.w * mat.m43,
		vec.x * mat.m14 + vec.y * mat.m24 + vec.z * mat.m34 + vec.w * mat.m44
	);
}


void Matrix44::SetOrthogonal(float left, float right, float bottom, float top, float near, float far, float offset, bool homogenDepth)
{
	memory::Set(this, 0, sizeof(float) * 16);

	const float aa = 2.0f / (right - left);
	const float bb = 2.0f / (top - bottom);
	const float cc = (homogenDepth ? 2.0f : 1.0f) / (far - near);
	const float dd = (left + right) / (left - right);
	const float ee = (top + bottom) / (bottom - top);
	const float ff = homogenDepth ? (near + far) / (near - far) : near / (near - far);

	m11 = aa;
	m22 = bb;
	m33 = -cc;
	m41 = dd + offset;
	m42 = ee;
	m43 = ff;
	m44 = 1.0f;
}


void Matrix44::SetPerspective(float fovY, float ratio, float near, float far, bool homogenDepth)
{
	memory::Set(this, 0, sizeof(float) * 16);

	float f = 1.0f / tanf(fovY * 0.5f);
	float zDiffInv = 1.0f / (far - near);

	m11 = f / ratio;
	m22 = f;
	m33 = ((homogenDepth) ? (far + near) : (far)) * zDiffInv;
	m44 = 0;
	//m43 = (homogenDepth) ? (2.0f * far * near * zDiffInv) : (near * far * zDiffInv);
	//m34 = /*-*/1.0f;
	m34 = 1.0f;
	m43 = -((homogenDepth) ? 2.0f : 1.0f) * near * far * zDiffInv;
}


void Matrix44::SetLookAt(const Vector4& eye, const Vector4& at, const Vector4& up)
{
	Vector3 eyeV3 = eye.GetXYZ();

	Vector3 dir = at.GetXYZ() - eyeV3;
	dir.Normalize();

	Vector3 right = Vector3::Cross(dir, up.GetXYZ());
	right.Normalize();

	Vector3 upNew = Vector3::Cross(right, dir);
	right.Normalize();

	memory::Set(this, 0, sizeof(float) * 16);

	m11 = right.x;
	m12 = upNew.x;
	m13 = dir.x;

	m21 = right.y;
	m22 = upNew.y;
	m23 = dir.y;

	m31 = right.z;
	m32 = upNew.z;
	m33 = dir.z;

	m41 = -Vector3::Dot(right, eyeV3);
	m42 = -Vector3::Dot(upNew, eyeV3);
	m43 = -Vector3::Dot(dir, eyeV3);
	m44 = 1.0f;
}


void Matrix44::SetRotateX(float angle)
{
	*this = IDENTITY;

	m22 = cosf(angle);
	m23 = -sinf(angle);
	m32 = sinf(angle);
	m33 = cosf(angle);
}

void Matrix44::SetRotateY(float angle)
{
	*this = IDENTITY;

	m11 = cosf(angle);
	m13 = sinf(angle);
	m31 = -sinf(angle);
	m33 = cosf(angle);
}

void Matrix44::SetRotateZ(float angle)
{
	*this = IDENTITY;

	m11 = cosf(angle);
	m12 = -sinf(angle);
	m21 = sinf(angle);
	m22 = cosf(angle);
}


void Matrix44::SetRotation(const Quaternion& quat)
{
	memory::Set(&m11, 0, 16 * sizeof(float));

	float fx = quat.x + quat.x;
	float fy = quat.y + quat.y;
	float fz = quat.z + quat.z;
	float fwx = fx * quat.w;
	float fwy = fy * quat.w;
	float fwz = fz * quat.w;
	float fxx = fx * quat.x;
	float fxy = fy * quat.x;
	float fxz = fz * quat.x;
	float fyy = fy * quat.y;
	float fyz = fz * quat.y;
	float fzz = fz * quat.z;

	m11 = 1.0f - (fyy + fzz);
	m12 = fxy - fwz;
	m13 = fxz + fwy;
	m21 = fxy + fwz;
	m22 = 1.0f - (fxx + fzz);
	m23 = fyz - fwx;
	m31 = fxz - fwy;
	m32 = fyz + fwx;
	m33 = 1.0f - (fxx + fyy);

	m44 = 1.0f;
}


void Matrix44::RotateX(float angle)
{
	Matrix44 rot;
	rot.SetRotateX(angle);
	*this = *this * rot;
}

void Matrix44::RotateY(float angle)
{
	Matrix44 rot;
	rot.SetRotateY(angle);
	*this = *this * rot;
}

void Matrix44::RotateZ(float angle)
{
	Matrix44 rot;
	rot.SetRotateZ(angle);
	*this = *this * rot;
}


void Matrix44::SetTranslation(const Vector3& trans)
{
	memory::Set(&m11, 0, 16 * sizeof(float));

	m11 = 1.0f;
	m22 = 1.0f;
	m33 = 1.0f;
	m41 = trans.x;
	m42 = trans.y;
	m43 = trans.z;
	m44 = 1.0f;
}


void Matrix44::SetScale(float scale)
{
	memory::Set(&m11, 0, 16 * sizeof(float));

	m11 = scale;
	m22 = scale;
	m33 = scale;
	m44 = 1.0f;
}


void Matrix44::Transpose()
{
	float tmp;
	tmp = m12; m12 = m21; m21 = tmp;
	tmp = m13; m13 = m31; m31 = tmp;
	tmp = m14; m14 = m41; m41 = tmp;

	tmp = m23; m23 = m32; m32 = tmp;
	tmp = m24; m24 = m42; m42 = tmp;

	tmp = m34; m34 = m43; m43 = tmp;
}


Matrix44 operator*(const Matrix44& mat1, const Matrix44& mat2)
{
	return Matrix44::Multiply(mat1, mat2);
}

Vector4 operator*(const Matrix44& mat, const Vector4& vec)
{
	return Matrix44::Multiply(mat, vec);
}

Vector4 operator*(const Vector4& vec, const Matrix44& mat)
{
	return Matrix44::Multiply(vec, mat);
}



Transform::Transform()
{

}

Transform::Transform(const Quaternion& rot, const Vector3& pos, float scale)
	: rotation(rot)
	, position(pos)
	, scale(scale)
{}

Transform::Transform(const Transform& other)
	: rotation(other.rotation)
	, position(other.position)
	, scale(other.scale)
{}

Matrix44 Transform::ToMatrix44() const
{
	Matrix44 rot;
	rot.SetRotation(rotation);
	Matrix44 trans;
	trans.SetTranslation(position);
	Matrix44 scal;
	scal.SetScale(scale);
	return scal * rot * trans;
}


}
