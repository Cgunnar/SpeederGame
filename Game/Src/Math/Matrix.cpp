#include "pch.hpp"
#include "Matrix.hpp"
#include "Transform.hpp"
#include <DirectXMath.h>
using namespace DirectX;

namespace Rimfrost
{
	XMMATRIX getXMMatrix(const Matrix& matrix)
	{
		return XMMATRIX((float*)(&matrix[0]));
	}

	Matrix::Matrix(float* mem)
	{
		memcpy(this, mem, 64);
	}

	Matrix::Matrix(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23, float _30, float _31, float _32, float _33)
	{
		columns[0][0] = _00; columns[0][1] = _01; columns[0][2] = _02; columns[0][3] = _03;
		columns[1][0] = _10; columns[1][1] = _11; columns[1][2] = _12; columns[1][3] = _13;
		columns[2][0] = _20; columns[2][1] = _21; columns[2][2] = _22; columns[2][3] = _23;
		columns[3][0] = _30; columns[3][1] = _31; columns[3][2] = _32; columns[3][3] = _33;
	}


	Matrix::Matrix(Vector4 columnVectors[4])
	{
		columns[0] = columnVectors[0];
		columns[1] = columnVectors[1];
		columns[2] = columnVectors[2];
		columns[3] = columnVectors[3];
	}

	Matrix::Matrix(float FovY, float aspectRatio, float nearPlane, float farPlane)
	{
		DirectX::XMFLOAT4X4 perspectiveMatrix;
		DirectX::XMStoreFloat4x4(&perspectiveMatrix, DirectX::XMMatrixPerspectiveFovLH(FovY, aspectRatio, nearPlane, farPlane));
		*this = Matrix((float*)&perspectiveMatrix);
	}




	
	Matrix transpose(const Matrix& m)
	{
		Matrix t;
		for(int col = 0; col < 4; col++)
		{
			for (int row = 0; row < 4; row++)
			{
				t[row][col] = m[col][row];
			}
		}
		return t;
	}
	Matrix inverse(const Matrix& matrix)
	{
		XMMATRIX inverse((float*)(&matrix[0]));
		XMVECTOR det = XMMatrixDeterminant(inverse);
		XMFLOAT4X4 f4x4invers;
		XMStoreFloat4x4(&f4x4invers, XMMatrixInverse(&det, inverse));
		
		return Matrix((float*)f4x4invers.m);
	}
	std::tuple<Matrix, Matrix, Matrix> decomposeToTRS(const Matrix& matrix)
	{
		XMVECTOR scale, rotationQuat, translation;
		XMMatrixDecompose(&scale, &rotationQuat, &translation, getXMMatrix(matrix));

		XMFLOAT4X4 S, R, T;
		XMStoreFloat4x4(&S, { XMMatrixScalingFromVector(scale) });
		XMStoreFloat4x4(&R, { XMMatrixRotationQuaternion(rotationQuat) });
		XMStoreFloat4x4(&T, { XMMatrixTranslationFromVector(translation) });

		return std::make_tuple(Matrix((float*)T.m), Matrix((float*)R.m), Matrix((float*)S.m));
	}
	Matrix scaleMatrix(float x, float y, float z)
	{
		Matrix s;
		s[0][0] = x;
		s[1][1] = y;
		s[2][2] = z;
		return s;
	}
	Matrix rotationFromAngles(float x, float y, float z)
	{
		return rotationZFromAngles(z) * rotationYFromAngles(y) * rotationXFromAngles(x);
	}
	Matrix rotationXFromAngles(float a)
	{
		Matrix rotX;
		rotX[1][1] = cosf(a);
		rotX[1][2] = sinf(a);
		rotX[2][1] = -sinf(a);
		rotX[2][2] = cosf(a);
		return rotX;
	}
	Matrix rotationYFromAngles(float a)
	{
		Matrix rotY;
		rotY[0][0] = cosf(a);
		rotY[0][2] = -sinf(a);
		rotY[2][0] = sinf(a);
		rotY[2][2] = cosf(a);
		return rotY;
	}
	Matrix rotationZFromAngles(float a)
	{
		Matrix rotZ;
		rotZ[0][0] = cosf(a);
		rotZ[0][1] = sinf(a);
		rotZ[1][0] = -sinf(a);
		rotZ[1][1] = cosf(a);
		return rotZ;
	}
	Matrix rotationFromAnglesDeg(float x, float y, float z)
	{
		return rotationZFromAnglesDeg(z) * rotationYFromAnglesDeg(y) * rotationXFromAnglesDeg(x);
	}
	Matrix rotationXFromAnglesDeg(float a)
	{
		a = XMConvertToRadians(a);
		return rotationXFromAngles(a);
	}
	Matrix rotationYFromAnglesDeg(float a)
	{
		a = XMConvertToRadians(a);
		return rotationYFromAngles(a);
	}
	Matrix rotationZFromAnglesDeg(float a)
	{
		a = XMConvertToRadians(a);
		return rotationZFromAngles(a);
	}
	Matrix rotationMatrixFromNormal(Vector3 normal, float angle)
	{
		assert(abs(normal.length() - 1) < 0.00001f);
		XMVECTOR XMnormal{ normal.x, normal.y, normal.z };
		XMFLOAT4X4 XMrot;
		XMStoreFloat4x4(&XMrot, DirectX::XMMatrixRotationNormal(XMnormal, angle));

		Matrix rot = Matrix((float*)XMrot.m); //transpose???
		return rot;
	}
	Matrix operator*(const Matrix& l, const Matrix& r)
	{
		Matrix lt = transpose(l);
		Matrix result;
	
		for (int i = 0; i < 4; i++)
		{
			result[i] = Vector4(dot4(lt[0], r[i]), dot4(lt[1], r[i]), dot4(lt[2], r[i]), dot4(lt[3], r[i]));
		}
		return result;
	}

	Matrix operator+(const Matrix& l, const Matrix& r)
	{
		Matrix sum;
		sum[0] = l[0] + r[0];
		sum[1] = l[1] + r[1];
		sum[2] = l[2] + r[2];
		sum[3] = l[3] + r[3];
		return sum;
	}

	Matrix operator-(const Matrix& l, const Matrix& r)
	{
		Matrix diff;
		diff[0] = l[0] - r[0];
		diff[1] = l[1] - r[1];
		diff[2] = l[2] - r[2];
		diff[3] = l[3] - r[3];
		return diff;
	}
	

	

	Vector4& Matrix::operator[](int index) noexcept
	{
		return columns[index];
	}

	const Vector4& Matrix::operator[](int index) const noexcept
	{
		return columns[index];
	}


	//matrix vector operation-----------------------------------


	Vector4 operator*(const Matrix& m, const Vector4& v)
	{
		Matrix t = transpose(m);
		return Vector4(dot4(t[0], v), dot4(t[1], v), dot4(t[2], v), dot4(t[3], v));
	}

}