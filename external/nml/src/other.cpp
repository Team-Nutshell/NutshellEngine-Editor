#include "../include/other.h"

float nml::toRad(const float degrees) {
	return degrees * (PI / 180.0f);
}

float nml::toDeg(const float radians) {
	return radians * (180.0f / PI);
}

float nml::lerp(const float a, const float b, const float interpolationValue) {
	return a + interpolationValue * (b - a);
}