#pragma once

#include <ostream>

namespace mtd
{
	constexpr float PI = 3.1415926536f;

	/*
	* @brief Representation of a 2D vector.
	*/
	struct Vec2
	{
		float x;
		float y;

		float& operator[](int i);
		float operator[](int i) const;

		Vec2 operator+(const Vec2& other) const;
		Vec2& operator+=(const Vec2& other);

		Vec2 operator-() const;
		Vec2 operator-(const Vec2& other) const;
		Vec2& operator-=(const Vec2& other);

		Vec2 operator*(float scalar) const;
		Vec2 operator*(const Vec2& other) const;
		Vec2& operator*=(float scalar);
		Vec2& operator*=(const Vec2& other);

		Vec2 operator/(float scalar) const;
		Vec2& operator/=(float scalar);

		friend std::ostream& operator<<(std::ostream& os, const Vec2& v2);

		/*
		* @brief Builds a 2D vector by specifying both X and Y values.
		*
		* @param x Value for the first element (X axis).
		* @param y Value for the second element (Y axis).
		*/
		constexpr Vec2(float x, float y) : x{x}, y{y} {}

		/*
		* @brief Calculates the dot product between this `Vec2` and a second `Vec2`.
		*
		* @param other Second operand of the dot product.
		*
		* @return Result of the dot product.
		*/
		float dot(const Vec2& other) const;

		/*
		* @brief Calculates the length of the vector.
		*
		* @return The length of the 2D vector.
		*/
		float length() const;

		/*
		* @brief Calculates the normalized value of the vector.
		*
		* @return The normalized 2D vector.
		*/
		Vec2 normalized() const;
	};

	/*
	* @brief Representation of a 3D vector.
	*/
	struct Vec3
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
			};
			struct
			{
				float r;
				float g;
				float b;
			};
		};

		float& operator[](int i);
		float operator[](int i) const;

		Vec3 operator+(const Vec3& other) const;
		Vec3& operator+=(const Vec3& other);

		Vec3 operator-() const;
		Vec3 operator-(const Vec3& other) const;
		Vec3& operator-=(const Vec3& other);

		Vec3 operator*(float scalar) const;
		Vec3 operator*(const Vec3& other) const;
		Vec3& operator*=(float scalar);
		Vec3& operator*=(const Vec3& other);

		Vec3 operator/(float scalar) const;
		Vec3& operator/=(float scalar);

		friend std::ostream& operator<<(std::ostream& os, const Vec3& v3);

		/*
		* @brief Builds a 3D vector by specifying all three values.
		*
		* @param x Value for the first element (X axis).
		* @param y Value for the second element (Y axis).
		* @param z Value for the third element (Z axis).
		*/
		constexpr Vec3(float x, float y, float z) : x{x}, y{y}, z{z} {}
		/*
		* @brief Builds a 3D vector from a 2D vector plus a scalar.
		*
		* @param vec2 2D vector with the two first values (X and Y axes).
		* @param z Scalar value with the third value (Z axis).
		*/
		constexpr Vec3(Vec2 vec2, float z) : x{vec2.x}, y{vec2.y}, z{z} {}

		/*
		* @brief Calculates the dot product between this `Vec3` and a second `Vec3`.
		*
		* @param other Second operand of the dot product.
		*
		* @return Result of the dot product.
		*/
		float dot(const Vec3& other) const;

		/*
		* @brief Calculates the cross product between this `Vec3` and a second three dimensional vector.
		*
		* @param other Second operand of the cross product.
		*
		* @return Result of the cross product.
		*/
		Vec3 cross(const Vec3& other) const;

		/*
		* @brief Calculates the length of the vector.
		*
		* @return The length of the 3D vector.
		*/
		float length() const;

		/*
		* @brief Calculates the normalized value of the vector.
		*
		* @return The normalized 3D vector.
		*/
		Vec3 normalized() const;
	};

	/*
	* @brief Representation of a 4D vector.
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

		Vec4 operator+(const Vec4& other) const;
		Vec4& operator+=(const Vec4& other);

		Vec4 operator-() const;
		Vec4 operator-(const Vec4& other) const;
		Vec4& operator-=(const Vec4& other);

		Vec4 operator*(float scalar) const;
		Vec4 operator*(const Vec4& other) const;
		Vec4& operator*=(float scalar);
		Vec4& operator*=(const Vec4& other);

		Vec4 operator/(float scalar) const;
		Vec4& operator/=(float scalar);

		friend std::ostream& operator<<(std::ostream& os, const Vec4& v4);

		/*
		* @brief Builds a 4D vector by specifying all four values.
		*
		* @param x Value for the first element.
		* @param y Value for the second element.
		* @param z Value for the third element.
		* @param w Value for the fourth element.
		*/
		constexpr Vec4(float x, float y, float z, float w) : x{x}, y{y}, z{z}, w{w} {}
		/*
		* @brief Builds a 4D vector from a 2D vector plus two scalars.
		*
		* @param vec2 2D vector with the two first elements (X and Y axes).
		* @param z Scalar value with the third element (Z axis).
		* @param w Scalar value with the fourth element (W axis).
		*/
		constexpr Vec4(Vec2 vec2, float z, float w) : x{vec2.x}, y{vec2.y}, z{z}, w{w} {}
		/*
		* @brief Builds a 4D vector from a 3D vector plus a scalar.
		*
		* @param vec3 3D vector with the three first elements (X, Y and Z axes).
		* @param w Scalar value with the fourth element (W axis).
		*/
		constexpr Vec4(Vec3 vec3, float w) : x{vec3.x}, y{vec3.y}, z{vec3.z}, w{w} {}
		/*
		* @brief Builds a 4D vector from two 2D vectors.
		*
		* @param xy 2D vector with the two first elements (X and Y axes).
		* @param zw 2D vector with the two last elements (Z and W axes).
		*/
		constexpr Vec4(Vec2 xy, Vec2 zw) : x{xy.x}, y{xy.y}, z{zw.x}, w{zw.y} {}

		/*
		* @brief Calculates the dot product between this `Vec4` and a second `Vec4`.
		*
		* @param other Second operand of the dot product.
		*
		* @return Result of the dot product.
		*/
		float dot(const Vec4& other) const;

		/*
		* @brief Calculates the length of the vector.
		*
		* @return The length of the 4D vector.
		*/
		float length() const;

		/*
		* @brief Calculates the normalized value of the vector.
		*
		* @return The normalized 4D vector.
		*/
		Vec4 normalized() const;
	};

	/*
	* @brief Representation of a quaternion.
	*/
	struct Quaternion
	{
		float w;
		float x;
		float y;
		float z;

		Quaternion operator+(const Quaternion& other) const;
		Quaternion& operator+=(const Quaternion& other);

		Quaternion operator-(const Quaternion& other) const;
		Quaternion& operator-=(const Quaternion& other);

		Quaternion operator*(float scalar) const;
		Quaternion operator*(const Quaternion& other) const;
		Vec3 operator*(const Vec3& vec) const;
		Quaternion& operator*=(float scalar);
		Quaternion& operator*=(const Quaternion& other);

		Quaternion operator/(float scalar) const;
		Quaternion& operator/=(float scalar);

		friend std::ostream& operator<<(std::ostream& os, const Quaternion& quat);

		/*
		* @brief Builds a quaternion by specifying all four elements.
		*
		* @param w Real component.
		* @param x Imaginary `i` component.
		* @param y Imaginary `j` component.
		* @param z Imaginary `k` component.
		*/
		constexpr Quaternion(float w, float x, float y, float z) : w{w}, x{x}, y{y}, z{z} {}

		/*
		* @brief Builds a quaternion from an axis and an angle.
		*
		* @param axis Main axis of rotation.
		* @param angle Rotation angle, in radians.
		*/
		Quaternion(float angle, const Vec3& axis);

		/*
		* @brief Gets the imaginary portion of the quaternion (xyz elements), as a 3D vector.
		*
		* @return Imaginary component of the quaternion.
		*/
		constexpr Vec3 imaginary() const { return Vec3{x, y, z}; }

		/*
		* @brief Calculates the norm of the quaternion.
		*
		* @return The value of the quaternion's norm.
		*/
		float norm() const;

		/*
		* @brief Calculates the conjugate of the quaternion.
		*
		* @return The conjugated quaternion.
		*/
		Quaternion conjugated() const;

		/*
		* @brief Calculates the normalized value of the quaternion.
		*
		* @return The normalized quaternion.
		*/
		Quaternion normalized() const;
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

		Mat4x4 operator*(float scalar) const;
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
		Mat4x4(const Quaternion& quat);

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

	Vec2 operator*(float scalar, const Vec2& vec);
	Vec3 operator*(float scalar, const Vec3& vec);
	Vec4 operator*(float scalar, const Vec4& vec);
	Mat4x4 operator*(float scalar, const Mat4x4& mat);
}
