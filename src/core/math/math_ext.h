#pragma once

#include "vector.h"
#include "matrix.h"
#include "quaternion.h"


namespace Veng
{


bool IsPointInsideTriangle(const Vector3& point, const Vector3& v1, const Vector3& v2, const Vector3& v3);


}