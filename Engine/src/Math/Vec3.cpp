#include <pch.hpp>
#include <meltdown/math.hpp>

mtd::Vec3::Vec3(float x, float y, float z) : x{x}, y{y}, z{z}
{
}

float& mtd::Vec3::operator[](int i)
{
	return (&x)[i];
}

float mtd::Vec3::operator[](int i) const
{
	return (&x)[i];
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
