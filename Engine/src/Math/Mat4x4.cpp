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
{}

mtd::Mat4x4::Mat4x4(float value)
	: x{value, 0.0f, 0.0f, 0.0f},
	y{0.0f, value, 0.0f, 0.0f},
	z{0.0f, 0.0f, value, 0.0f},
	w{0.0f, 0.0f, 0.0f, value}
{}

mtd::Mat4x4::Mat4x4(const Quaternion& quat)
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
{}

mtd::Vec4& mtd::Mat4x4::operator[](size_t i)
{
	assert(i < 4UL && "Mat4x4 index out of bounds.");
	return (&x)[i];
}

const mtd::Vec4& mtd::Mat4x4::operator[](size_t i) const
{
	assert(i < 4UL && "Mat4x4 index out of bounds.");
	return (&x)[i];
}

mtd::Mat4x4 mtd::Mat4x4::operator*(float scalar) const
{
	return
	{
		x.x * scalar, x.y * scalar, x.z * scalar, x.w * scalar,
		y.x * scalar, y.y * scalar, y.z * scalar, y.w * scalar,
		z.x * scalar, z.y * scalar, z.z * scalar, z.w * scalar,
		w.x * scalar, w.y * scalar, w.z * scalar, w.w * scalar
	};
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

mtd::Mat4x4 mtd::operator*(float scalar, const Mat4x4& mat)
{
	return
	{
		mat.x.x * scalar, mat.x.y * scalar, mat.x.z * scalar, mat.x.w * scalar,
		mat.y.x * scalar, mat.y.y * scalar, mat.y.z * scalar, mat.y.w * scalar,
		mat.z.x * scalar, mat.z.y * scalar, mat.z.z * scalar, mat.z.w * scalar,
		mat.w.x * scalar, mat.w.y * scalar, mat.w.z * scalar, mat.w.w * scalar
	};
}

float mtd::Mat4x4::determinant() const
{
	float t0 = x.x * y.y - y.x * x.y;
	float t1 = x.x * y.z - y.x * x.z;
	float t2 = x.x * y.w - y.x * x.w;
	float t3 = x.y * y.z - y.y * x.z;
	float t4 = x.y * y.w - y.y * x.w;
	float t5 = x.z * y.w - y.z * x.w;

	float b0 = z.x * w.y - w.x * z.y;
	float b1 = z.x * w.z - w.x * z.z;
	float b2 = z.x * w.w - w.x * z.w;
	float b3 = z.y * w.z - w.y * z.z;
	float b4 = z.y * w.w - w.y * z.w;
	float b5 = z.z * w.w - w.z * z.w;

	return t0 * b5 - t1 * b4 + t2 * b3 + t3 * b2 - t4 * b1 + t5 * b0;
}

mtd::Mat4x4 mtd::Mat4x4::inverse() const
{
	float t0 = x.x * y.y - y.x * x.y;
	float t1 = x.x * y.z - y.x * x.z;
	float t2 = x.x * y.w - y.x * x.w;
	float t3 = x.y * y.z - y.y * x.z;
	float t4 = x.y * y.w - y.y * x.w;
	float t5 = x.z * y.w - y.z * x.w;

	float b0 = z.x * w.y - w.x * z.y;
	float b1 = z.x * w.z - w.x * z.z;
	float b2 = z.x * w.w - w.x * z.w;
	float b3 = z.y * w.z - w.y * z.z;
	float b4 = z.y * w.w - w.y * z.w;
	float b5 = z.z * w.w - w.z * z.w;

	float determinant = t0 * b5 - t1 * b4 + t2 * b3 + t3 * b2 - t4 * b1 + t5 * b0;
	if(std::abs(determinant) < 1e-8f)
		return Mat4x4{0.0f};

	float inverseDeterminant = 1.0f / determinant;

	Mat4x4 inv
	{
		{
			y.y * b5 - y.z * b4 + y.w * b3,
			-x.y * b5 + x.z * b4 - x.w * b3,
			w.y * t5 - w.z * t4 + w.w * t3,
			-z.y * t5 + z.z * t4 - z.w * t3
		},
		{
			-y.x * b5 + y.z * b2 - y.w * b1,
			x.x * b5 - x.z * b2 + x.w * b1,
			-w.x * t5 + w.z * t2 - w.w * t1,
			z.x * t5 - z.z * t2 + z.w * t1
		},
		{
			y.x * b4 - y.y * b2 + y.w * b0,
			-x.x * b4 + x.y * b2 - x.w * b0,
			w.x * t4 - w.y * t2 + w.w * t0,
			-z.x * t4 + z.y * t2 - z.w * t0
		},
		{
			-y.x * b3 + y.y * b1 - y.z * b0,
			x.x * b3 - x.y * b1 + x.z * b0,
			-w.x * t3 + w.y * t1 - w.z * t0,
			z.x * t3 - z.y * t1 + z.z * t0
		}
	};

	return inverseDeterminant * inv;
}

void mtd::Mat4x4::rotateIntrinsic(float angle, const Vec3& axis)
{
	Mat4x4 rotationMatrix{Quaternion{angle, axis}};

	*this *= rotationMatrix;
}

void mtd::Mat4x4::rotateExtrinsic(float angle, const Vec3& axis)
{
	Vec3 offset{this->w.x, this->w.y, this->w.z};
	this->w.x = 0.0f;
	this->w.y = 0.0f;
	this->w.z = 0.0f;

	Mat4x4 rotationMatrix{Quaternion{angle, axis}};

	*this = rotationMatrix * *this;
	this->w.x = offset.x;
	this->w.y = offset.y;
	this->w.z = offset.z;
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
