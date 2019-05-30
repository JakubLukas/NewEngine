#include "math.h"
#include <cmath>


namespace Veng
{


float copysignf(float num, float sign)
{
	return ::copysignf(num, sign);
}


float absf(float num)
{
	return ::fabsf(num);
}


float sinf(float num)
{
	return ::sinf(num);
}

float asinf(float num)
{
	return ::asinf(num);
}


float cosf(float num)
{
	return ::cosf(num);
}

float acosf(float num)
{
	return ::acosf(num);
}


float tanf(float num)
{
	return ::tanf(num);
}

float atanf(float num)
{
	return ::atanf(num);
}

float atan2f(float y, float x)
{
	return ::atan2f(y, x);
}


float powf(float num, float exp)
{
	return ::powf(num, exp);
}

float sqrtf(float num)
{
	return ::sqrtf(num);
}


}
