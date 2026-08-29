#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include <cstdint>
#include <cstring>

#include "core/Common.h"
#include "Graphics.h"
#include "Shader.h"

namespace Dive
{
	template<class T>
	class ConstantBuffer
	{
	public:
		ConstantBuffer(Graphics* graphics);
		~ConstantBuffer() = default;

		void Update(Graphics* graphics, const T& bufferData);

		void Bind(Graphics* graphics, eShaderStage stage, uint32_t slot);

		ID3D11Buffer* GetBuffer() const { return m_buffer.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
	};

	template<class T>
	ConstantBuffer<T>::ConstantBuffer(Graphics* graphics)
	{
		D3D11_BUFFER_DESC desc{};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.ByteWidth = sizeof(T);

		auto device = graphics->GetDevice();
		HRESULT hr = device->CreateBuffer(&desc, nullptr, m_buffer.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("ConstantBuffer 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			assert(false && "ConstantBuffe 생성 실패!");
		}
	}

	template<class T>
	void ConstantBuffer<T>::Update(Graphics* graphics, const T& bufferData)
	{
		graphics->UpdateBuffer(m_buffer.Get(), &bufferData, static_cast<uint32_t>(sizeof(T)));
	}

	template<class T>
	void ConstantBuffer<T>::Bind(Graphics* graphics, eShaderStage stage, uint32_t slot)
	{
		graphics->SetConstantBuffer(stage, slot, m_buffer.GetAddressOf());
	}
}