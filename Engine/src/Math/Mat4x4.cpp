#include <pch.hpp>
#include <meltdown/math.hpp>

mtd::Mat4x4::Mat4x4
(
	float xx, float xy, float xz, float xw,
	float yx, float yy, float yz, float yw,
	float zx, float zy, float zz, float zw,
	float wx, float wy, float wz, float ww
) : x{xx, xy, xz, xw},
	y{yx, yy, yz, yw},
	z{zx, zy, zz, zw},
	w{wx, wy, wz, ww}
{
}

mtd::Mat4x4::Mat4x4(float value)
	: x{value, 0.0f, 0.0f, 0.0f},
	y{0.0f, value, 0.0f, 0.0f},
	z{0.0f, 0.0f, value, 0.0f},
	w{0.0f, 0.0f, 0.0f, value}
{
}

mtd::Mat4x4::Mat4x4(const Vec4& quat)
	: x
	{
		1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z),
		2.0f * (quat.x * quat.y - quat.z * quat.w),
		2.0f * (quat.x * quat.z + quat.y * quat.w),
		0.0f
	},
	y
	{
		2.0f * (quat.x * quat.y + quat.z * quat.w),
		1.0f - 2.0f * (quat.x * quat.x + quat.z * quat.z),
		2.0f * (quat.y * quat.z - quat.x * quat.w),
		0.0f
	},
	z
	{
		2.0f * (quat.x * quat.z - quat.y * quat.w),
		2.0f * (quat.y * quat.z + quat.x * quat.w),
		1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y),
		0.0f
	},
	w{0.0f, 0.0f, 0.0f, 1.0f}
{
}

mtd::Vec4& mtd::Mat4x4::operator[](int i)
{
	return (&x)[i];
}

const mtd::Vec4& mtd::Mat4x4::operator[](int i) const
{
	return (&x)[i];
}

mtd::Mat4x4 mtd::Mat4x4::operator*(const Mat4x4& other) const
{
	glm::mat4 result{*((glm::mat4*)this) * *((glm::mat4*)(&other))};
	return *((Mat4x4*)(&result));
}

mtd::Mat4x4& mtd::Mat4x4::operator*=(const Mat4x4& other)
{
	glm::mat4 result{*((glm::mat4*)this) * *((glm::mat4*)(&other))};
	return *this = *((Mat4x4*)(&result));
}

namespace mtd
{
	std::ostream& operator<<(std::ostream& os, const Mat4x4& mat)
	{
		for(uint32_t i = 0; i < 4; i++)
		{
			os << "\n[ ";
			for(uint32_t j = 0; j < 4; j++)
				os << std::fixed << std::setw(8) << std::setprecision(3) << mat[i][j] << "  ";
			os << ']';
		}
		os << '\n';

		return os;
	}
}

void mtd::Mat4x4::rotateIntrinsic(float angle, const Vec3& axis)
{
	Mat4x4 rotationMatrix{Vec4{axis, angle}};

	*this *= rotationMatrix;
}

void mtd::Mat4x4::rotateExtrinsic(float angle, const Vec3& axis)
{
	Vec3 offset{this->w.x, this->w.y, this->w.z};
	this->w.x = 0.0f;
	this->w.y = 0.0f;
	this->w.z = 0.0f;

	Mat4x4 rotationMatrix{Vec4{axis, angle}};

	*this = rotationMatrix * *this;
	this->w.x = offset.x;
	this->w.y = offset.y;
	this->w.z = offset.z;
}
