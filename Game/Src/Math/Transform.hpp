#pragma once

#include "Matrix.hpp"

namespace Rimfrost
{
	class Transform
	{
	public:
		Transform() = default;
		~Transform() = default;

		Transform(const Matrix& matrix);

		operator Matrix& () { return m_matrix; }
		operator const Matrix& () const { return m_matrix; } 

		void setTranslation(float x, float y, float z);
		void setTranslation(const Vector3& position);
		void translate(float x, float y, float z);
		void translate(const Vector3& position);

		void setRotation(float x, float y, float z);
		void setRotation(const Matrix& rotationMatrix);
		void setRotationDeg(float x, float y, float z);
		void rotate(float x, float y, float z);
		void rotate(const Matrix& rotationMatrix);
		void rotateDeg(float x, float y, float z);

		void setScale(float x, float y, float z);
		void setScale(float scale);
		void scale(float x, float y, float z);
		void scale(float scale);

		Vector3 getTranslation() const;

		Matrix getRotationMatrix() const;

		Vector3 forward() const;
		Vector3 up() const;
		Vector3 right() const;



	private:
		Matrix m_matrix;
	};
}
