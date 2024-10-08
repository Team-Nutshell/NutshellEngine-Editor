#include "../include/vec3.h"
#include "../include/vec2.h"
#include "../include/vec4.h"
#include "../include/mat4.h"
#include "../include/quat.h"
#include <cmath>
#include <stdexcept>

namespace nml {

vec3::vec3(): x(0.0f), y(0.0f), z(0.0f) {}
vec3::vec3(float _value): x(_value), y(_value), z(_value) {}
vec3::vec3(float _x, float _y, float _z): x(_x), y(_y), z(_z) {}
vec3::vec3(float _x, vec2 _yz): x(_x), y(_yz.x), z(_yz.y) {}
vec3::vec3(vec2 _xy, float _z): x(_xy.x), y(_xy.y), z(_z) {}
vec3::vec3(const float* _ptr): x(*_ptr), y(*(_ptr + 1)), z(*(_ptr + 2)) {}
vec3::vec3(vec4 _xyzw): x(_xyzw.x), y(_xyzw.y), z(_xyzw.z) {}

vec3& vec3::operator+=(const vec3& other) { 
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

vec3& vec3::operator-=(const vec3& other) { 
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

vec3& vec3::operator*=(const float other) {
	x *= other;
	y *= other;
	z *= other;

	return *this;
}

vec3& vec3::operator/=(const float other) {
	x /= other;
	y /= other;
	z /= other;

	return *this;
}

vec3 vec3::operator-() const {
	return vec3(-x, -y, -z);
}

float& vec3::operator[](size_t index) {
	if (index == 0) { return x; }
	else if (index == 1) { return y; }
	else if (index == 2) { return z; }
	else { throw std::out_of_range("vec3::operator[]: index is out of range."); }
}

const float vec3::operator[](size_t index) const {
	if (index == 0) { return x; }
	else if (index == 1) { return y; }
	else if (index == 2) { return z; }
	else { throw std::out_of_range("vec3::operator[]: index is out of range."); }
}

float vec3::length() const {
	return std::sqrt((x * x) + (y * y) + (z * z));
}

float* vec3::data() {
	return &x;
}

vec3 operator+(vec3 lhs, const vec3& rhs) { 
	lhs += rhs;

	return lhs;
}

vec3 operator-(vec3 lhs, const vec3& rhs) {
	lhs -= rhs;

	return lhs;
}

vec3 operator*(vec3 lhs, const float rhs) {
	lhs *= rhs;

	return lhs;
}

vec3 operator*(float lhs, const vec3& rhs) {
	return (rhs * lhs);
}

vec3 operator/(vec3 lhs, const float rhs) { 
	lhs /= rhs;

	return lhs;
}

bool operator==(const vec3& lhs, const vec3& rhs) {
	return ((lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z));
}

bool operator!=(const vec3& lhs, const vec3& rhs) {
	return !(lhs == rhs);
}

vec3 normalize(const vec3& vec) {
	const float l = vec.length();

	return (vec / l);
}

float dot(const vec3& a, const vec3& b) {
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

vec3 cross(const vec3& a, const vec3& b) {
	return vec3(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

vec3 reflect(const vec3& i, const vec3& n) {
	return (i - 2.0f * dot(n, i) * n);
}

vec3 refract(const vec3& i, const vec3& n, float ior) {
	const float ndoti = dot(n, i);
	const float k = 1.0f - ior * ior * (1.0f - ndoti * ndoti);
	if (k < 0.0f) {
		return vec3(0.0f);
	}
	else {
		return ior * i - (ior * ndoti + std::sqrt(k)) * n;
	}
}

vec3 quatToEulerAngles(const quat& qua) {
	const mat4 rotationMatrix = quatToRotationMatrix(qua);

	return rotationMatrixToEulerAngles(rotationMatrix);
}

vec3 rotationMatrixToEulerAngles(const mat4& mat) {
	return vec3(-std::atan2(mat.z.y, mat.z.z),
		-std::atan2(-mat.z.x, std::sqrt((mat.z.y * mat.z.y) + (mat.z.z * mat.z.z))),
		-std::atan2(mat.y.x, mat.x.x));
}

std::string to_string(const vec3& vec) {
	return ("[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + "]");
}

}