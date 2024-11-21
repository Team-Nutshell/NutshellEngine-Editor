#pragma once
#include <string>

namespace nml {

struct vec3;
struct mat4;

// a + bi + cj + dk
struct quat {
    float a;
    float b;
    float c;
    float d;

    // Constructors
    quat();
    quat(float _a, float _b, float _c, float _d);
    quat(const float* _ptr);

    // Operators
    quat& operator+=(const quat& other);
    quat& operator-=(const quat& other);
    quat& operator*=(const quat& other);
    quat& operator*=(const float other);
	quat& operator/=(const float other);
	quat operator-() const;
    float& operator[](size_t index);
	const float operator[](size_t index) const;

    // Functions
    float length() const;

    float* data();

	// Static Functions
	static quat identity();
};

// Operators
quat operator+(quat lhs, const quat& rhs);
quat operator-(quat lhs, const quat& rhs);
quat operator*(quat lhs, const quat& rhs);
quat operator*(quat lhs, const float rhs);
quat operator*(float lhs, const quat& rhs);
quat operator/(quat lhs, const float rhs);
bool operator==(const quat& lhs, const quat& rhs);
bool operator!=(const quat& lhs, const quat& rhs);

// Functions
quat conjugate(const quat& qua);
quat normalize(const quat& qua);
float dot(const quat& a, const quat& b);
quat slerp(const quat& a, const quat& b, const float interpolationValue);
quat eulerAnglesToQuat(const vec3& vec);
quat rotationMatrixToQuat(const mat4& mat);

std::string to_string(const quat& qua);

}