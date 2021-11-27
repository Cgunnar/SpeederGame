
#include "Vector.hpp"
#include <cmath>
#include <assert.h>
#include <DirectXMath.h>

namespace Rimfrost
{


	Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
	{
	}
	Vector4::Vector4(const Vector3& v3, float w)
	{
		this->x = v3.x;
		this->y = v3.y;
		this->z = v3.z;
		this->w = w;
	}
	float& Vector4::operator[](int index) noexcept
	{
		switch (index)
		{
		case 0:	return this->x;
		case 1:	return this->y;
		case 2: return this->z;
		case 3: return this->w;
		default:
			assert(false);
			return this->x;
		}
	}
	float Vector4::operator[](int index) const noexcept
	{
		switch (index)
		{
		case 0:	return this->x;
		case 1:	return this->y;
		case 2: return this->z;
		case 3: return this->w;
		default:
			assert(false);
			return this->x;
		}
	}
	float Vector4::length2() const
	{
		return sqrt(x * x + y * y);
	}
	float Vector4::length3() const
	{
		return sqrt(x * x + y * y + z * z);
	}
	float Vector4::length4() const
	{
		return sqrt(x * x + y * y + z * z + w * w);
	}
	float dot2(const Vector4& v, const Vector4& u)
	{
		return v.x * u.x + v.y * u.y;
	}
	float dot3(const Vector4& v, const Vector4& u)
	{
		return v.x * u.x + v.y * u.y + v.z * u.z;
	}
	float dot4(const Vector4& v, const Vector4& u)
	{
		return v.x * u.x + v.y * u.y + v.z * u.z + v.w * u.w;
	}
	Vector4 operator+(const Vector4& l, const Vector4& r)
	{
		return Vector4(l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w);
	}
	Vector4 operator-(const Vector4& l, const Vector4& r)
	{
		return Vector4(l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w);
	}


	//Vector3-------------------------------------------

	Vector3::Vector3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	Vector3::Vector3(const Vector4& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}
	Vector3::Vector3(const Vector2& v, float z) : x(v.x), y(v.y), z(z)
	{
	}
	float& Vector3::operator[](int index) noexcept
	{
		switch (index)
		{
		case 0:	return this->x;
		case 1:	return this->y;
		case 2: return this->z;
		default:
			assert(false);
			return this->x;
		}
	}
	Vector3& Vector3::operator+=(const Vector3& other)
	{
		this->x = this->x + other.x;
		this->y = this->y + other.y;
		this->z = this->z + other.z;
		return *this;
	}
	float Vector3::length() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	void Vector3::normalize()
	{

		float f = length();
		if (f == 0) return;
		f = 1.0f / f;
		*this = f * *this;
	}

	float dot(const Vector3& v, const Vector3& u)
	{
		return v.x * u.x + v.y * u.y + v.z * u.z;
	}

	Vector3 cross(const Vector3& v, const Vector3& u)
	{
		return { v.y * u.z - v.z * u.y, v.z * u.x - v.x * u.z, v.x * u.y - v.y * u.x };
	}

	Vector3 normalize(const Vector3& v)
	{
		float f = v.length();
		if (f == 0)
		{
			assert(false);
			return Vector3();
		};

		f = 1.0f / f;
		return f * v;
	}

	Vector3 operator+(const Vector3& l, const Vector3& r)
	{
		return Vector3(l.x + r.x, l.y + r.y, l.z + r.z);
	}
	Vector4 operator-(const Vector4& l, const Vector3& r)
	{
		return Vector4(l.x - r.x, l.y - r.y, l.z - r.z, l.w);
	}
	Vector4 operator-(const Vector3& l, const Vector4& r)
	{
		return Vector4(l.x - r.x, l.y - r.y, l.z - r.z, r.w);
	}
	Vector4 operator+(const Vector4& l, const Vector3& r)
	{
		return Vector4(l.x + r.x, l.y + r.y, l.z + r.z, l.w);
	}
	Vector4 operator+(const Vector3& l, const Vector4& r)
	{
		return r + l;
	}
	Vector3 operator-(const Vector3& l, const Vector3& r)
	{
		return Vector3(l.x - r.x, l.y - r.y, l.z - r.z);
	}

	Vector3 operator*(const Vector3& l, const Vector3& r)
	{
		return Vector3(l.x * r.x, l.y * r.y, l.z * r.z);
	}

	Vector3 operator*(float scale, const Vector3& v)
	{
		return Vector3(scale * v.x, scale * v.y, scale * v.z);
	}

	Vector3 operator/(const Vector3& v, float scale)
	{
		return Vector3(v.x / scale, v.y / scale, v.z / scale);
	}

	float dot(const Vector2& v, const Vector2& u)
	{
		return v.x * u.x + v.y * u.y;
	}

	Vector2 operator+(const Vector2& l, const Vector2& r)
	{
		return Vector2(l.x + r.x, l.y + r.y);
	}

	Vector2 operator-(const Vector2& l, const Vector2& r)
	{
		return Vector2(l.x - r.x, l.y - r.y);
	}

	Vector2 operator*(float scale, const Vector2& v)
	{
		return Vector2(scale * v.x, scale * v.y);
	}

	Vector2 operator/(const Vector2& v, float scale)
	{
		return Vector2(v.x / scale, v.y / scale);
	}

	Vector2::Vector2(float x, float y) : x(x), y(y)
	{
	}

	Vector2::Vector2(const Vector3& v) : x(v.x), y(v.y)
	{
	}

	float& Vector2::operator[](int index) noexcept
	{
		switch (index)
		{
		case 0:	return this->x;
		case 1:	return this->y;
		default:
			assert(false);
			return this->x;
		}
	}

	Vector2& Vector2::operator+=(const Vector2& other)
	{
		this->x = this->x + other.x;
		this->y = this->y + other.y;
		return *this;
	}

	float Vector2::length() const
	{
		return sqrt(x * x + y * y);
	}

	void Vector2::normalize()
	{
		float f = length();
		if (f == 0) return;
		f = 1.0f / f;
		*this = f * *this;
	}

}