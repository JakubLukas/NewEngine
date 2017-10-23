#include "matrix.h"


namespace Veng
{


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


}