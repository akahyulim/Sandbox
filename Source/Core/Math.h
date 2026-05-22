#pragma once
#include <DirectXMath.h>

namespace Dive
{
	class Math
	{
	public:
		static bool XMMatrixEqual(const DirectX::XMMATRIX& a, const DirectX::XMMATRIX& b);
		static bool XMMatrixNearEqual(const DirectX::XMMATRIX& a, const DirectX::XMMATRIX& b, float epsilon = 1e-5f);

		static bool XMFLOAT3Equal(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, float epsilon = 1e-6f);
		static bool XMFLOAT4Equal(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b, float epsilon = 1e-6f);
		static bool XMFLOAT4X4Equal(const DirectX::XMFLOAT4X4& a, const DirectX::XMFLOAT4X4& b, float epsilon = 1e-6f);

		static DirectX::XMFLOAT3 CalcuBiTangent(const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& tangent);

		static DirectX::XMFLOAT3 QuaternionToDegrees(const DirectX::XMFLOAT4& quaternion);
		static DirectX::XMFLOAT4 DegreesToQuaternion(const DirectX::XMFLOAT3& degrees);
		static DirectX::XMFLOAT3 QuaternionToRadians(const DirectX::XMFLOAT4& quaternion);
		static DirectX::XMFLOAT4 RadiansToQuaternion(const DirectX::XMFLOAT3& radians);

		static DirectX::XMFLOAT3 GetPositionFromTransform(const DirectX::XMFLOAT4X4& transform);
		static DirectX::XMFLOAT4 GetRotationFromTransform(const DirectX::XMFLOAT4X4& transform);
		static DirectX::XMFLOAT3 GetScaleFromTransform(const DirectX::XMFLOAT4X4& transform);
	};
}