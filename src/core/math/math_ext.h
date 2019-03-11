#pragma once

#include "vector.h"
#include "matrix.h"
#include "quaternion.h"


namespace Veng
{


Vector3 CortesianToBarycentric(const Vector3& point, const Vector3& v1, const Vector3& v2, const Vector3& v3);

bool IsPointInsideTriangle(const Vector3& point, const Vector3& v1, const Vector3& v2, const Vector3& v3);


}