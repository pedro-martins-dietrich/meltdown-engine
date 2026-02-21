#include <pch.hpp>
#include <meltdown/math.hpp>

uint32_t& mtd::UIntVec2::operator[](size_t i)
{
	assert(i < 2UL && "UIntVec2 index out of bounds.");
	return (&x)[i];
}

uint32_t mtd::UIntVec2::operator[](size_t i) const
{
	assert(i < 2UL && "UIntVec2 index out of bounds.");
	return (&x)[i];
}

mtd::UIntVec2 mtd::UIntVec2::operator+(UIntVec2 other) const
{
	return {x + other.x, y + other.y};
}

mtd::UIntVec2& mtd::UIntVec2::operator+=(UIntVec2 other)
{
	x += other.x;
	y += other.y;
	return *this;
}

mtd::UIntVec2 mtd::UIntVec2::operator-() const
{
	return {-x, -y};
}

mtd::UIntVec2 mtd::UIntVec2::operator-(UIntVec2 other) const
{
	return {x - other.x, y - other.y};
}

mtd::UIntVec2& mtd::UIntVec2::operator-=(UIntVec2 other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

mtd::UIntVec2 mtd::UIntVec2::operator*(uint32_t scalar) const
{
	return {x * scalar, y * scalar};
}

mtd::UIntVec2 mtd::operator*(uint32_t scalar, UIntVec2 vec)
{
	return {vec.x * scalar, vec.y * scalar};
}

mtd::UIntVec2 mtd::UIntVec2::operator*(UIntVec2 other) const
{
	return {x * other.x, y * other.y};
}

mtd::UIntVec2& mtd::UIntVec2::operator*=(uint32_t scalar)
{
	x *= scalar;
	y *= scalar;
	return *this;
}

mtd::UIntVec2& mtd::UIntVec2::operator*=(UIntVec2 other)
{
	x *= other.x;
	y *= other.y;
	return *this;
}

mtd::UIntVec2 mtd::UIntVec2::operator/(uint32_t scalar) const
{
	return {x / scalar, y / scalar};
}

mtd::UIntVec2& mtd::UIntVec2::operator/=(uint32_t scalar)
{
	x /= scalar;
	y /= scalar;
	return *this;
}

namespace mtd
{
	std::ostream& operator<<(std::ostream& os, const UIntVec2& uv2)
	{
		os << '(' << uv2.x << ", " << uv2.y << ')';
		return os;
	}
}
