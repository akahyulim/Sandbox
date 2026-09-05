#include "pch.h"
#include "Shader.h"
#include "Core/Common.h"
#include "Graphics.h"

namespace Dive
{
	VertexShader::VertexShader(Graphics* graphics, ID3DBlob* code)
	{
		auto device = graphics->GetDevice();
		auto hr = device->CreateVertexShader(
			code->GetBufferPointer(), 
			code->GetBufferSize(), 
			nullptr, 
			m_vs.GetAddressOf()); 
		if (FAILED(hr))
		{
			spdlog::error("VertexShader 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			assert(false && "VertexShader 생성 실패!");
		}
	}

	VertexShader::~VertexShader() = default;
	
	PixelShader::PixelShader(Graphics* graphics, ID3DBlob* code)
	{
		auto device = graphics->GetDevice();
		auto hr = device->CreatePixelShader(
			code->GetBufferPointer(),
			code->GetBufferSize(),
			nullptr, 
			m_ps.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("PixelShader 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			assert(false && "PixelShader 생성 실패!");
		}
	}
	
	PixelShader::~PixelShader() = default;

	ComputeShader::ComputeShader(Graphics* graphics, ID3DBlob* code)
	{
		auto device = graphics->GetDevice();
		auto hr = device->CreateComputeShader(
			code->GetBufferPointer(),
			code->GetBufferSize(),
			nullptr,
			m_cs.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("ComputeShader 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			assert(false && "ComputeShader 생성 실패!");
		}
	}

	ComputeShader::~ComputeShader() = default;
}
