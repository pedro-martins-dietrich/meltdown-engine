#include <pch.hpp>
#include <meltdown/math.hpp>

float& mtd::Vec4::operator[](int i)
{
	return (&x)[i];
}

float mtd::Vec4::operator[](int i) const
{
	return (&x)[i];
}

mtd::Vec4 mtd::Vec4::operator+(const Vec4& other) const
{
	return {x + other.x, y + other.y, z + other.z, w + other.w};
}

mtd::Vec4& mtd::Vec4::operator+=(const Vec4& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	w += other.w;
	return *this;
}

mtd::Vec4 mtd::Vec4::operator-() const
{
	return {-x, -y, -z, -w};
}

mtd::Vec4 mtd::Vec4::operator-(const Vec4& other) const
{
	return {x - other.x, y - other.y, z - other.z, w - other.w};
}

mtd::Vec4& mtd::Vec4::operator-=(const Vec4& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	w -= other.w;
	return *this;
}

mtd::Vec4 mtd::Vec4::operator*(float scalar) const
{
	return {x * scalar, y * scalar, z * scalar, w * scalar};
}

mtd::Vec4 mtd::operator*(float scalar, const Vec4& vec)
{
	return {vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar};
}

mtd::Vec4 mtd::Vec4::operator*(const Vec4& other) const
{
	return {x * other.x, y * other.y, z * other.z, w * other.w};
}

mtd::Vec4& mtd::Vec4::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

mtd::Vec4& mtd::Vec4::operator*=(const Vec4& other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	w *= other.w;
	return *this;
}

mtd::Vec4 mtd::Vec4::operator/(float scalar) const
{
	float inverse = 1.0f / scalar;
	return {x * inverse, y * inverse, z * inverse, w * inverse};
}

mtd::Vec4& mtd::Vec4::operator/=(float scalar)
{
	float inverse = 1.0f / scalar;
	x *= inverse;
	y *= inverse;
	z *= inverse;
	w *= inverse;
	return *this;
}

namespace mtd
{
	std::ostream& operator<<(std::ostream& os, const Vec4& v4)
	{
		os << '(' << std::fixed;
		for(uint32_t i = 0; i < 4; i++)
			os << std::setw(8) << std::setprecision(3) << v4[i] << ((i % 4 == 3) ? ")" : ", ");
		return os;
	}
}

float mtd::Vec4::dot(const Vec4& other) const
{
	return x * other.x + y * other.y + z * other.z + w * other.w;
}

float mtd::Vec4::length() const
{
	return sqrtf(x * x + y * y + z * z + w * w);
}

mtd::Vec4 mtd::Vec4::normalized() const
{
	return *this / length();
}
