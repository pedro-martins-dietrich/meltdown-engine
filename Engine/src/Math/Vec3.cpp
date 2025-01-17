#include <pch.hpp>
#include <meltdown/math.hpp>

float& mtd::Vec3::operator[](int i)
{
	return (&x)[i];
}

float mtd::Vec3::operator[](int i) const
{
	return (&x)[i];
}

mtd::Vec3 mtd::Vec3::operator+(const Vec3& other) const
{
	return {x + other.x, y + other.y, z + other.z};
}

mtd::Vec3& mtd::Vec3::operator+=(const Vec3& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

mtd::Vec3 mtd::Vec3::operator-(const Vec3& other) const
{
	return {x - other.x, y - other.y, z - other.z};
}

mtd::Vec3& mtd::Vec3::operator-=(const Vec3& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

mtd::Vec3 mtd::Vec3::operator*(float scalar) const
{
	return {x * scalar, y * scalar, z * scalar};
}

mtd::Vec3 mtd::operator*(float scalar, const Vec3& vec)
{
	return {vec.x * scalar, vec.y * scalar, vec.z * scalar};
}

mtd::Vec3 mtd::Vec3::operator*(const Vec3& other) const
{
	return {x * other.x, y * other.y, z * other.z};
}

mtd::Vec3& mtd::Vec3::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

mtd::Vec3& mtd::Vec3::operator*=(const Vec3& other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	return *this;
}

mtd::Vec3 mtd::Vec3::operator/(float scalar) const
{
	float inverse = 1.0f / scalar;
	return {x * inverse, y * inverse, z * inverse};
}

mtd::Vec3& mtd::Vec3::operator/=(float scalar)
{
	float inverse = 1.0f / scalar;
	x *= inverse;
	y *= inverse;
	z *= inverse;
	return *this;
}

namespace mtd
{
	std::ostream& operator<<(std::ostream& os, const Vec3& v3)
	{
		os << '(';
		for(uint32_t i = 0; i < 3; i++)
			os << std::setw(8) << std::setprecision(3) << v3[i] << ((i % 3 == 2) ? ")" : ", ");
		return os;
	}
}

float mtd::Vec3::dot(const Vec3& other) const
{
	return x * other.x + y * other.y + z * other.z;
}

mtd::Vec3 mtd::Vec3::cross(const Vec3& other) const
{
	return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
}

float mtd::Vec3::length() const
{
	return sqrtf(x * x + y * y + z * z);
}

mtd::Vec3 mtd::Vec3::normalized() const
{
	return *this / length();
}
