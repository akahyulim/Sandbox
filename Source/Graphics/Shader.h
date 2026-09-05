#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DXGI1_3.h>

namespace Dive
{
	enum class eShaderStage
	{
		VS,
		PS,
		HS,
		DS,
		GS,
		CS,
		Count
	};

	class Graphics;

	class Shader
	{
	public:
		virtual ~Shader() = default;

	protected:
		Shader() = default;
	}; 

	class VertexShader : public Shader
	{
	public:
		VertexShader(Graphics* graphics, ID3DBlob* code);
		~VertexShader() override;

		operator ID3D11VertexShader* () const { return m_vs.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
	};

	class PixelShader : public Shader
	{
	public:
		PixelShader(Graphics* graphics, ID3DBlob* code);
		~PixelShader() override;

		operator ID3D11PixelShader* () const { return m_ps.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps;
	};

	class ComputeShader : public Shader
	{
	public:
		ComputeShader(Graphics* graphics, ID3DBlob* code);
		~ComputeShader() override;

		operator ID3D11ComputeShader* () const { return m_cs.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_cs;
	};
}
