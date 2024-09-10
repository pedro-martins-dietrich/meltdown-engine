#include <meltdown/math.hpp>

#include <cmath>
#include <iomanip>

mtd::Vec4::Vec4(float x, float y, float z, float w) : x{x}, y{y}, z{z}, w{w}
{
}

mtd::Vec4::Vec4(const Vec3& axis, float angle)
{
	float halfAngle = 0.5f * angle;
	float angleSin = std::sin(halfAngle);

	x = axis.x * angleSin;
	y = axis.y * angleSin;
	z = axis.z * angleSin;
	w = std::cos(halfAngle);
}

float& mtd::Vec4::operator[](int i)
{
	return (&x)[i];
}

float mtd::Vec4::operator[](int i) const
{
	return (&x)[i];
}

std::ostream& mtd::operator<<(std::ostream& os, const Vec4& v4)
{
	os << '(';
	for(uint32_t i = 0; i < 4; i++)
		os << std::setw(8) << std::setprecision(3) << v4[i] << ((i % 4 == 3) ? ")" : ", ");
	return os;
}
