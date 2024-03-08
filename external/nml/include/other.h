#pragma once

namespace nml {

const float PI = 3.1415926535897932384626433832795f;

float toRad(const float degrees);
float toDeg(const float radians);

float lerp(const float a, const float b, const float interpolationValue);

}