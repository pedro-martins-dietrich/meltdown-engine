#include <pch.hpp>
#include <meltdown/math.hpp>

float& mtd::Vec2::operator[](int i)
{
	return (&x)[i];
}

float mtd::Vec2::operator[](int i) const
{
	return (&x)[i];
}

mtd::Vec2 mtd::Vec2::operator+(const Vec2& other) const
{
	return {x + other.x, y + other.y};
}

mtd::Vec2& mtd::Vec2::operator+=(const Vec2& other)
{
	x += other.x;
	y += other.y;
	return *this;
}

mtd::Vec2 mtd::Vec2::operator-(const Vec2& other) const
{
	return {x - other.x, y - other.y};
}

mtd::Vec2& mtd::Vec2::operator-=(const Vec2& other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

mtd::Vec2 mtd::Vec2::operator*(float scalar) const
{
	return {x * scalar, y * scalar};
}

mtd::Vec2 mtd::operator*(float scalar, const Vec2& vec)
{
	return {vec.x * scalar, vec.y * scalar};
}

mtd::Vec2 mtd::Vec2::operator*(const Vec2& other) const
{
	return {x * other.x, y * other.y};
}

mtd::Vec2& mtd::Vec2::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	return *this;
}

mtd::Vec2& mtd::Vec2::operator*=(const Vec2& other)
{
	x *= other.x;
	y *= other.y;
	return *this;
}

mtd::Vec2 mtd::Vec2::operator/(float scalar) const
{
	float inverse = 1.0f / scalar;
	return {x * inverse, y * inverse};
}

mtd::Vec2& mtd::Vec2::operator/=(float scalar)
{
	float inverse = 1.0f / scalar;
	x *= inverse;
	y *= inverse;
	return *this;
}

namespace mtd
{
	std::ostream& operator<<(std::ostream& os, const Vec2& v2)
	{
		os << '(' << std::setw(8) << std::setprecision(3) << v2.x << ", ";
		os << std::setw(8) << std::setprecision(3) << v2.y << ')';
		return os;
	}
}

float mtd::Vec2::dot(const Vec2& other) const
{
	return x * other.x + y * other.y;
}

float mtd::Vec2::length() const
{
	return sqrtf(x * x + y * y);
}

mtd::Vec2 mtd::Vec2::normalized() const
{
	return *this / length();
}
