#include "../include/quat.h"
#include "../include/vec3.h"
#include <cmath>
#include <stdexcept>

namespace nml {

quat::quat(): a(0.0f), b(0.0f), c(0.0f), d(0.0f) {}
quat::quat(float _a, float _b, float _c, float _d): a(_a), b(_b), c(_c), d(_d) {}
quat::quat(const float* _ptr): a(*_ptr), b(*(_ptr + 1)), c(*(_ptr + 2)), d(*(_ptr + 3)) {}

quat& quat::operator+=(const quat& other) {
	a += other.a;
	b += other.b;
	c += other.c;
	d += other.d;

	return *this;
}

quat& quat::operator-=(const quat& other) {
	a -= other.a;
	b -= other.b;
	c -= other.c;
	d -= other.d;

	return *this;
}

quat& quat::operator*=(const quat& other) {
	const quat tmp((a * other.a) - (b * other.b) - (c * other.c) - (d * other.d),
	(a * other.b) + (b * other.a) + (c * other.d) - (d * other.c),
	(a * other.c) - (b * other.d) + (c * other.a) + (d * other.b),
	(a * other.d) + (b * other.c) - (c * other.b) + (d * other.a));

	a = tmp.a;
	b = tmp.b;
	c = tmp.c;
	d = tmp.d;

	return *this;
}

quat& quat::operator*=(const float other) {
	a *= other;
	b *= other;
	c *= other;
	d *= other;

	return *this;
}

quat& quat::operator/=(const float other) {
	a /= other;
	b /= other;
	c /= other;
	d /= other;

	return *this;
}

quat quat::operator-() const {
	return quat(-a, -b, -c, -d);
}

float& quat::operator[](size_t index) {
	if (index == 0) { return a; }
	else if (index == 1) { return b; }
	else if (index == 2) { return c; }
	else if (index == 3) { return d; }
	else { throw std::out_of_range("quat::operator[]: index is out of range."); }
}

const float quat::operator[](size_t index) const {
	if (index == 0) { return a; }
	else if (index == 1) { return b; }
	else if (index == 2) { return c; }
	else if (index == 3) { return d; }
	else { throw std::out_of_range("quat::operator[]: index is out of range."); }
}

float quat::length() const {
	return std::sqrt((a * a) + (b * b) + (c * c) + (d * d));
}

float* quat::data() {
	return &a;
}

quat quat::identity() {
	return quat(1.0f, 0.0f, 0.0f, 0.0f);
}

quat operator+(quat lhs, const quat& rhs) {
	lhs += rhs;

	return lhs;
}

quat operator-(quat lhs, const quat& rhs) {
	lhs -= rhs;

	return lhs;
}

quat operator*(quat lhs, const quat& rhs) {
	lhs *= rhs;

	return lhs;
}

quat operator*(quat lhs, const float rhs) {
	lhs *= rhs;

	return lhs;
}

quat operator*(float lhs, const quat& rhs) {
	return (rhs * lhs);
}

quat operator/(quat lhs, const float rhs) {
	lhs /= rhs;

	return lhs;
}

bool operator==(const quat& lhs, const quat& rhs) {
	return ((lhs.a == rhs.a) && (lhs.b == rhs.b) && (lhs.c == rhs.c) && (lhs.d == rhs.d));
}

bool operator!=(const quat& lhs, const quat& rhs) {
	return !(lhs == rhs);
}

quat conjugate(const quat& qua) {
	return quat(qua.a, -qua.b, -qua.c, -qua.d);
}

quat normalize(const quat& qua) {
	const float l = qua.length();

	return (qua / l);
}

float dot(const quat& a, const quat& b) {
	return ((a.a * b.a) + (a.b * b.b) + (a.c * b.c) + (a.d * b.d));
}

quat slerp(const quat& a, const quat& b, const float interpolationValue) {
	quat tmpB = b;

	float aDotb = dot(a, b);

	if (aDotb < 0.0) {
		tmpB = tmpB * -1.0f;
		aDotb = -aDotb;
	}

	if (aDotb > 0.9995) {
		return nml::normalize(a + interpolationValue * (tmpB - a));
	}

	const float theta0 = std::acos(aDotb);
	const float theta = interpolationValue * theta0;
	const float sinTheta0 = std::sin(theta0);
	const float sinTheta = std::sin(theta);

	float scaleA = std::cos(theta) - aDotb * (sinTheta / sinTheta0);
	float scaleB = sinTheta / sinTheta0;

	return (scaleA * a + scaleB * tmpB);
}

quat eulerAnglesToQuat(const vec3& vec) {
	const float cosHalfX = std::cos(vec.x / 2.0f);
	const float sinHalfX = std::sin(vec.x / 2.0f);
	const float cosHalfY = std::cos(vec.y / 2.0f);
	const float sinHalfY = std::sin(vec.y / 2.0f);
	const float cosHalfZ = std::cos(vec.z / 2.0f);
	const float sinHalfZ = std::sin(vec.z / 2.0f);

	return quat(cosHalfX * cosHalfY * cosHalfZ - sinHalfX * sinHalfY * sinHalfZ,
		sinHalfX * cosHalfY * cosHalfZ + cosHalfX * sinHalfY * sinHalfZ,
		cosHalfX * sinHalfY * cosHalfZ - sinHalfX * cosHalfY * sinHalfZ,
		cosHalfX * cosHalfY * sinHalfZ + sinHalfX * sinHalfY * cosHalfZ);
}

std::string to_string(const quat& qua) {
	return std::to_string(qua.a) + " + " + std::to_string(qua.b) + "i + " + std::to_string(qua.c) + "j + " + std::to_string(qua.d) + "k";
}

}