#include "math.h"
#include <math.h>


namespace Veng
{


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


float pow(float num, float exp)
{
	return ::powf(num, exp);
}

float sqrtf(float num)
{
	return ::sqrtf(num);
}


float min(float v1, float v2)
{
	return (v1 < v2) ? v1 : v2;
}

float max(float v1, float v2)
{
	return (v1 > v2) ? v1 : v2;
}


}
