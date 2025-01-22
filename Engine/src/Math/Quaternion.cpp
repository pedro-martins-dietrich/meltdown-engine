#include <pch.hpp>
#include <meltdown/math.hpp>

mtd::Quaternion::Quaternion(float angle, const Vec3& axis)
{
	float halfAngle = 0.5f * angle;
	float angleSin = sinf(halfAngle);

	x = axis.x * angleSin;
	y = axis.y * angleSin;
	z = axis.z * angleSin;
	w = cosf(halfAngle);
}

mtd::Quaternion mtd::Quaternion::operator+(const mtd::Quaternion& other) const
{
	return {w + other.w, x + other.x, y + other.y, z + other.z};
}

mtd::Quaternion& mtd::Quaternion::operator+=(const mtd::Quaternion& other)
{
	return *this = {w + other.w, x + other.x, y + other.y, z + other.z};
}

mtd::Quaternion mtd::Quaternion::operator-(const mtd::Quaternion& other) const
{
	return {w - other.w, x - other.x, y - other.y, z - other.z};
}

mtd::Quaternion& mtd::Quaternion::operator-=(const mtd::Quaternion& other)
{
	return *this = {w - other.w, x - other.x, y - other.y, z - other.z};
}

mtd::Quaternion mtd::Quaternion::operator*(float scalar) const
{
	return {w * scalar, x * scalar, y * scalar, z * scalar};
}

mtd::Quaternion mtd::Quaternion::operator*(const mtd::Quaternion& other) const
{
	return
	{
		w * other.w - x * other.x - y * other.y - z * other.z,
		w * other.x + x * other.w + y * other.z - z * other.y,
		w * other.y - x * other.z + y * other.w + z * other.x,
		w * other.z + x * other.y - y * other.x + z * other.w
	};
}

mtd::Vec3 mtd::Quaternion::operator*(const Vec3& vec) const
{
	return vec + 2.0f * imaginary().cross(imaginary().cross(vec) + w * vec);
}

mtd::Quaternion& mtd::Quaternion::operator*=(float scalar)
{
	return *this = {w * scalar, x * scalar, y * scalar, z * scalar};
}

mtd::Quaternion& mtd::Quaternion::operator*=(const mtd::Quaternion& other)
{
	return *this =
	{
		w * other.w - x * other.x - y * other.y - z * other.z,
		w * other.x + x * other.w + y * other.z - z * other.y,
		w * other.y - x * other.z + y * other.w + z * other.x,
		w * other.z + x * other.y - y * other.x + z * other.w
	};
}

mtd::Quaternion mtd::Quaternion::operator/(float scalar) const
{
	float inverse = 1.0f / scalar;
	return {w * inverse, x * inverse, y * inverse, z * inverse};
}

mtd::Quaternion& mtd::Quaternion::operator/=(float scalar)
{
	float inverse = 1.0f / scalar;
	return *this = {w * inverse, x * inverse, y * inverse, z * inverse};
}

namespace mtd
{
	std::ostream& operator<<(std::ostream& os, const Quaternion& quat)
	{
		os << '(' << std::fixed << std::setw(8) << std::setprecision(3) << quat.w << " + " <<
			std::setw(8) << std::setprecision(3) << quat.x << "i + " <<
			std::setw(8) << std::setprecision(3) << quat.y << "j + " <<
			std::setw(8) << std::setprecision(3) << quat.z << "k)";
		return os;
	}
}

float mtd::Quaternion::norm() const
{
	return sqrtf(w * w + x * x + y * y + z * z);
}

mtd::Quaternion mtd::Quaternion::conjugated() const
{
	return {w, -x, -y, -z};
}

mtd::Quaternion mtd::Quaternion::normalized() const
{
	return *this / norm();
}
