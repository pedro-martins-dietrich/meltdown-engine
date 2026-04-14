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

		float& operator[](size_t i);
		float operator[](size_t i) const;

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
		* @brief Builds a 2D vector by specifying a single value for both X and Y.
		*
		* @param value Value for both elements (X and Y axes).
		*/
		constexpr Vec2(float value) : x{value}, y{value} {}

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

		/*
		* @brief Clamps the vector's values between a minimum and a maximum, component-wise.
		*
		* @param minimum Vector containing the minimum value for each component.
		* @param maximum Vector containing the maximum value for each component.
		*
		* @return The clamped 2D vector.
		*/
		Vec2 clamp(Vec2 minimum, Vec2 maximum) const;
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

		float& operator[](size_t i);
		float operator[](size_t i) const;

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
		* @brief Builds a 3D vector by specifying a single value for all three components.
		*
		* @param value Value for all elements (X, Y and Z axes).
		*/
		constexpr Vec3(float value) : x{value}, y{value}, z{value} {}
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

		/*
		* @brief Clamps the vector's values between a minimum and a maximum, component-wise.
		*
		* @param minimum Vector containing the minimum value for each component.
		* @param maximum Vector containing the maximum value for each component.
		*
		* @return The clamped 3D vector.
		*/
		Vec3 clamp(const Vec3& minimum, const Vec3& maximum) const;
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

		float& operator[](size_t i);
		float operator[](size_t i) const;

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
		* @brief Builds a 4D vector by specifying a single value for all four components.
		*
		* @param value Value for all elements (X, Y, Z and W axes).
		*/
		constexpr Vec4(float value) : x{value}, y{value}, z{value}, w{value} {}
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

		/*
		* @brief Clamps the vector's values between a minimum and a maximum, component-wise.
		*
		* @param minimum Vector containing the minimum value for each component.
		* @param maximum Vector containing the maximum value for each component.
		*
		* @return The clamped 4D vector.
		*/
		Vec4 clamp(const Vec4& minimum, const Vec4& maximum) const;
	};

	/*
	* @brief Representation of a 2D vector of unsigned integers.
	*/
	struct UIntVec2
	{
		uint32_t x;
		uint32_t y;

		uint32_t& operator[](size_t i);
		uint32_t operator[](size_t i) const;

		UIntVec2 operator+(UIntVec2 other) const;
		UIntVec2& operator+=(UIntVec2 other);

		UIntVec2 operator-() const;
		UIntVec2 operator-(UIntVec2 other) const;
		UIntVec2& operator-=(UIntVec2 other);

		UIntVec2 operator*(uint32_t scalar) const;
		UIntVec2 operator*(UIntVec2 other) const;
		UIntVec2& operator*=(uint32_t scalar);
		UIntVec2& operator*=(UIntVec2 other);

		UIntVec2 operator/(uint32_t scalar) const;
		UIntVec2& operator/=(uint32_t scalar);

		friend std::ostream& operator<<(std::ostream& os, UIntVec2 uv2);

		/*
		* @brief Builds a 2D unsigned integer vector by specifying both values.
		*
		* @param x Value for the first element.
		* @param y Value for the second element.
		*/
		constexpr UIntVec2(uint32_t x, uint32_t y) : x{x}, y{y} {}
		/*
		* @brief Builds a 2D unsigned integer vector by specifying a single value for both components.
		*
		* @param value Unsigned integer value for both elements.
		*/
		constexpr UIntVec2(uint32_t value) : x{value}, y{value} {}

		/*
		* @brief Clamps the vector's values between a minimum and a maximum, component-wise.
		*
		* @param minimum Vector containing the minimum value for each component.
		* @param maximum Vector containing the maximum value for each component.
		*
		* @return The clamped 2D integer vector.
		*/
		UIntVec2 clamp(UIntVec2 minimum, UIntVec2 maximum) const;
	};

	/*
	* @brief Representation of a 3D vector of unsigned integers.
	*/
	struct UIntVec3
	{
		uint32_t x;
		uint32_t y;
		uint32_t z;

		uint32_t& operator[](size_t i);
		uint32_t operator[](size_t i) const;

		UIntVec3 operator+(UIntVec3 other) const;
		UIntVec3& operator+=(UIntVec3 other);

		UIntVec3 operator-() const;
		UIntVec3 operator-(UIntVec3 other) const;
		UIntVec3& operator-=(UIntVec3 other);

		UIntVec3 operator*(uint32_t scalar) const;
		UIntVec3 operator*(UIntVec3 other) const;
		UIntVec3& operator*=(uint32_t scalar);
		UIntVec3& operator*=(UIntVec3 other);

		UIntVec3 operator/(uint32_t scalar) const;
		UIntVec3& operator/=(uint32_t scalar);

		friend std::ostream& operator<<(std::ostream& os, const UIntVec3& uv3);

		/*
		* @brief Builds a 3D unsigned integer vector by specifying all three values.
		*
		* @param x Value for the first element.
		* @param y Value for the second element.
		* @param z Value for the third element.
		*/
		constexpr UIntVec3(uint32_t x, uint32_t y, uint32_t z) : x{x}, y{y}, z{z} {}
		/*
		* @brief Builds a 3D unsigned integer vector by specifying a single value for all three components.
		*
		* @param value Unsigned integer value for all elements.
		*/
		constexpr UIntVec3(uint32_t value) : x{value}, y{value}, z{value} {}
		/*
		* @brief Builds a 3D unsigned integer vector by specifying the two first values
		* with a 2D unsigned integer vector, and the last with an unsigned integer.
		*
		* @param xy Value for the two first elements.
		* @param z Value for the third element.
		*/
		constexpr UIntVec3(UIntVec2 xy, uint32_t z) : x{xy.x}, y{xy.y}, z{z} {}
		/*
		* @brief Builds a 3D unsigned integer vector by specifying the first value with
		* an unsigned integer, and the last two values with a 2D unsigned integer vector.
		*
		* @param x Value for the first element.
		* @param yz Value for the two last elements.
		*/
		constexpr UIntVec3(uint32_t x, UIntVec3 yz) : x{x}, y{yz.y}, z{yz.z} {}

		/*
		* @brief Clamps the vector's values between a minimum and a maximum, component-wise.
		*
		* @param minimum Vector containing the minimum value for each component.
		* @param maximum Vector containing the maximum value for each component.
		*
		* @return The clamped 3D integer vector.
		*/
		UIntVec3 clamp(const UIntVec3& minimum, const UIntVec3& maximum) const;
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

		Vec4& operator[](size_t i);
		const Vec4& operator[](size_t i) const;

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

	Vec2 operator*(float scalar, Vec2 vec);
	Vec3 operator*(float scalar, const Vec3& vec);
	Vec4 operator*(float scalar, const Vec4& vec);
	UIntVec2 operator*(uint32_t scalar, UIntVec2 vec);
	UIntVec3 operator*(uint32_t scalar, const UIntVec3& vec);
	Mat4x4 operator*(float scalar, const Mat4x4& mat);

	/*
	* @brief Calculates the component-wise minimum between two `Vec2` vectors.
	*
	* @param a First operand of the minimum operation.
	* @param b Second operand of the minimum operation.
	*
	* @return A `Vec2` vector with the minimum value for each component between the two operands.
	*/
	constexpr Vec2 min(Vec2 a, Vec2 b)
	{
		return Vec2{(a.x < b.x) ? a.x : b.x, (a.y < b.y) ? a.y : b.y};
	}
	/*
	* @brief Calculates the component-wise minimum between two `Vec3` vectors.
	*
	* @param a First operand of the minimum operation.
	* @param b Second operand of the minimum operation.
	*
	* @return A `Vec3` vector with the minimum value for each component between the two operands.
	*/
	constexpr Vec3 min(const Vec3& a, const Vec3& b)
	{
		return Vec3{(a.x < b.x) ? a.x : b.x, (a.y < b.y) ? a.y : b.y, (a.z < b.z) ? a.z : b.z};
	}
	/*
	* @brief Calculates the component-wise minimum between two `Vec4` vectors.
	*
	* @param a First operand of the minimum operation.
	* @param b Second operand of the minimum operation.
	*
	* @return A `Vec4` vector with the minimum value for each component between the two operands.
	*/
	constexpr Vec4 min(const Vec4& a, const Vec4& b)
	{
		return Vec4
		{
			(a.x < b.x) ? a.x : b.x, (a.y < b.y) ? a.y : b.y,
			(a.z < b.z) ? a.z : b.z, (a.w < b.w) ? a.w : b.w
		};
	}
	/*
	* @brief Calculates the component-wise minimum between two `UIntVec2` vectors.
	*
	* @param a First operand of the minimum operation.
	* @param b Second operand of the minimum operation.
	*
	* @return A `UIntVec2` vector with the minimum value for each component between the two operands.
	*/
	constexpr UIntVec2 min(UIntVec2 a, UIntVec2 b)
	{
		return UIntVec2{(a.x <= b.x) ? a.x : b.x, (a.y <= b.y) ? a.y : b.y};
	}
	/*
	* @brief Calculates the component-wise minimum between two `UIntVec3` vectors.
	*
	* @param a First operand of the minimum operation.
	* @param b Second operand of the minimum operation.
	*
	* @return A `UIntVec3` vector with the minimum value for each component between the two operands.
	*/
	constexpr UIntVec3 min(const UIntVec3& a, const UIntVec3& b)
	{
		return UIntVec3{(a.x <= b.x) ? a.x : b.x, (a.y <= b.y) ? a.y : b.y, (a.z <= b.z) ? a.z : b.z};
	}

	/*
	* @brief Calculates the component-wise maximum between two `Vec2` vectors.
	*
	* @param a First operand of the maximum operation.
	* @param b Second operand of the maximum operation.
	*
	* @return A `Vec2` vector with the maximum value for each component between the two operands.
	*/
	constexpr Vec2 max(Vec2 a, Vec2 b)
	{
		return Vec2{(a.x > b.x) ? a.x : b.x, (a.y > b.y) ? a.y : b.y};
	}
	/*
	* @brief Calculates the component-wise maximum between two `Vec3` vectors.
	*
	* @param a First operand of the maximum operation.
	* @param b Second operand of the maximum operation.
	*
	* @return A `Vec3` vector with the maximum value for each component between the two operands.
	*/
	constexpr Vec3 max(const Vec3& a, const Vec3& b)
	{
		return Vec3{(a.x > b.x) ? a.x : b.x, (a.y > b.y) ? a.y : b.y, (a.z > b.z) ? a.z : b.z};
	}
	/*
	* @brief Calculates the component-wise maximum between two `Vec4` vectors.
	*
	* @param a First operand of the maximum operation.
	* @param b Second operand of the maximum operation.
	*
	* @return A `Vec4` vector with the maximum value for each component between the two operands.
	*/
	constexpr Vec4 max(const Vec4& a, const Vec4& b)
	{
		return Vec4
		{
			(a.x > b.x) ? a.x : b.x, (a.y > b.y) ? a.y : b.y,
			(a.z > b.z) ? a.z : b.z, (a.w > b.w) ? a.w : b.w
		};
	}
	/*
	* @brief Calculates the component-wise maximum between two `UIntVec2` vectors.
	*
	* @param a First operand of the maximum operation.
	* @param b Second operand of the maximum operation.
	*
	* @return A `UIntVec2` vector with the maximum value for each component between the two operands.
	*/
	constexpr UIntVec2 max(UIntVec2 a, UIntVec2 b)
	{
		return UIntVec2{(a.x > b.x) ? a.x : b.x, (a.y > b.y) ? a.y : b.y};
	}
	/*
	* @brief Calculates the component-wise maximum between two `UIntVec3` vectors.
	*
	* @param a First operand of the maximum operation.
	* @param b Second operand of the maximum operation.
	*
	* @return A `UIntVec3` vector with the maximum value for each component between the two operands.
	*/
	constexpr UIntVec3 max(const UIntVec3& a, const UIntVec3& b)
	{
		return UIntVec3{(a.x > b.x) ? a.x : b.x, (a.y > b.y) ? a.y : b.y, (a.z > b.z) ? a.z : b.z};
	}
}
