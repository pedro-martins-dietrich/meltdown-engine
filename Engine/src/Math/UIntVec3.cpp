#include <pch.hpp>
#include <meltdown/math.hpp>

uint32_t& mtd::UIntVec3::operator[](size_t i)
{
	assert(i < 3UL && "UIntVec3 index out of bounds.");
	return (&x)[i];
}

uint32_t mtd::UIntVec3::operator[](size_t i) const
{
	assert(i < 3UL && "UIntVec3 index out of bounds.");
	return (&x)[i];
}

mtd::UIntVec3 mtd::UIntVec3::operator+(UIntVec3 other) const
{
	return {x + other.x, y + other.y, z + other.z};
}

mtd::UIntVec3& mtd::UIntVec3::operator+=(UIntVec3 other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

mtd::UIntVec3 mtd::UIntVec3::operator-() const
{
	return {-x, -y, -z};
}

mtd::UIntVec3 mtd::UIntVec3::operator-(UIntVec3 other) const
{
	return {x - other.x, y - other.y, z - other.z};
}

mtd::UIntVec3& mtd::UIntVec3::operator-=(UIntVec3 other)
{
	x -= other.x;
	y -= other.y;
	z -= other.x;
	return *this;
}

mtd::UIntVec3 mtd::UIntVec3::operator*(uint32_t scalar) const
{
	return {x * scalar, y * scalar, z * scalar};
}

mtd::UIntVec3 mtd::operator*(uint32_t scalar, const UIntVec3& vec)
{
	return {vec.x * scalar, vec.y * scalar, vec.z * scalar};
}

mtd::UIntVec3 mtd::UIntVec3::operator*(UIntVec3 other) const
{
	return {x * other.x, y * other.y, z * other.z};
}

mtd::UIntVec3& mtd::UIntVec3::operator*=(uint32_t scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

mtd::UIntVec3& mtd::UIntVec3::operator*=(UIntVec3 other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	return *this;
}

mtd::UIntVec3 mtd::UIntVec3::operator/(uint32_t scalar) const
{
	return {x / scalar, y / scalar, z / scalar};
}

mtd::UIntVec3& mtd::UIntVec3::operator/=(uint32_t scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	return *this;
}

std::ostream& mtd::operator<<(std::ostream& os, const UIntVec3& uv3)
{
	os << '(' << uv3.x << ", " << uv3.y << ", " << uv3.z << ')';
	return os;
}

mtd::UIntVec3 mtd::UIntVec3::clamp(const UIntVec3& minimum, const UIntVec3& maximum) const
{
	return max(minimum, min(*this, maximum));
}
