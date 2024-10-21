#pragma once

#include <ostream>

namespace mtd
{
	/*
	* @brief Representation of a 3D vector.
	*/
	struct Vec3
	{
		float x;
		float y;
		float z;

		float& operator[](int i);
		float operator[](int i) const;

		friend std::ostream& operator<<(std::ostream& os, const Vec3& v3);

		/*
		* @brief Builds a 3D vector by specifying all three values.
		*
		* @param x Value for the X axis.
		* @param y Value for the Y axis.
		* @param z Value for the Z axis.
		*/
		Vec3(float x, float y, float z);
	};

	/*
	* @brief Representation of a 4D vector.
	* Can also be used to represent a RGBA color or a quaternion.
	*/
	struct Vec4
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
				float w;
			};
			struct
			{
				float r;
				float g;
				float b;
				float a;
			};
		};

		float& operator[](int i);
		float operator[](int i) const;

		friend std::ostream& operator<<(std::ostream& os, const Vec4& v4);

		/*
		* @brief Builds a 4D vector by specifying all four values.
		*
		* @param x Value for the first element.
		* @param y Value for the second element.
		* @param z Value for the third element.
		* @param w Value for the fourth element.
		*/
		Vec4(float x, float y, float z, float w);
		/*
		* @brief Builds a quaternion from an axis and an angle.
		*
		* @param axis Main axis of rotation.
		* @param angle Rotation angle, in radians.
		*/
		Vec4(const Vec3& axis, float angle);
	};

	/*
	* @brief Representation of a 4x4 matrix.
	*/
	struct Mat4x4
	{
		Vec4 x;
		Vec4 y;
		Vec4 z;
		Vec4 w;

		Vec4& operator[](int i);
		const Vec4& operator[](int i) const;

		Mat4x4 operator*(const Mat4x4& other) const;
		Mat4x4& operator*=(const Mat4x4& other);

		friend std::ostream& operator<<(std::ostream& os, const Mat4x4& mat);

		/*
		* @brief Builds a 4x4 matrix by specifying all sixteen values.
		*/
		Mat4x4
		(
			float xx, float xy, float xz, float xw,
			float yx, float yy, float yz, float yw,
			float zx, float zy, float zz, float zw,
			float wx, float wy, float wz, float ww
		);
		/*
		* @brief Builds a scalar matrix.
		* Using `1.0f` as a value builds the identity matrix.
		*
		* @param value Scalar value for all elements where `i == j`.
		*/
		Mat4x4(float value);
		/*
		* @brief Builds a 4x4 rotation matrix from a quaternion.
		*
		* @param quat Quaternion that the describes the rotation the created matrix will perform.
		*/
		Mat4x4(const Vec4& quat);

		/*
		* @brief Rotates () the matrix around the axis, by an angle (in radians).
		* The rotation is done intrinsically (the rotation axis is defined based on the
		* model's current orientation).
		*
		* @param angle Rotation angle, in radians.
		* @param axis 3D vector defining the rotation axis, using the model's orthonormal base.
		*/
		void rotateIntrinsic(float angle, const Vec3& axis);

		/*
		* @brief Rotates the matrix around the axis, by an angle (in radians).
		* The rotation is done extrinsically (the rotation axis is defined based on the global
		* orientation).
		*
		* @param angle Rotation angle, in radians.
		* @param axis 3D vector defining the rotation axis, using the global orthonormal base.
		*/
		void rotateExtrinsic(float angle, const Vec3& axis);
	};
}
