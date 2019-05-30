#pragma once

#include "vector.h"
#include "matrix.h"
#include "quaternion.h"


namespace Veng
{


bool IsPointInsideTriangle(const Vector3& point, const Vector3& v1, const Vector3& v2, const Vector3& v3);


void ToRad(Vector3& angles);
void ToDeg(Vector3& angles);

// euler x: yaw, y: pitch, z: roll
void QuatToEuler(const Quaternion& quat, Vector3& euler);

// euler x: yaw, y: pitch, z: roll
void EulerToQuat(const Vector3& euler, Quaternion& quat);


}