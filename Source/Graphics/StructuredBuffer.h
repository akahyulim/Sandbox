#pragma once
#include <d3d11.h>
#include <spdlog/spdlog.h>

#include "Graphics.h"

namespace Dive
{
	enum class eStructuredBufferType
	{
		Read,
		Write,
		ReadWrite
	};

	template<class T>
	class StructuredBuffer
	{
	public:
		StructuredBuffer(Graphics* graphics, eStructuredBufferType type, const T* initialData = nullptr, uint32_t elementCount = 1);
		~StructuredBuffer() = default;

		ID3D11Buffer* GetBuffer() const { return m_buffer.Get(); }
		ID3D11UnorderedAccessView* GetUAV() const { return m_uav.Get(); }
		ID3D11ShaderResourceView* GetSRV() const { return m_srv.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_uav;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
	};

	template<class T>
	StructuredBuffer<T>::StructuredBuffer(Graphics* graphics, eStructuredBufferType type, const T* initialData, uint32_t elementCount)
	{
		D3D11_BUFFER_DESC desc{};
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.CPUAccessFlags = 0;
		desc.ByteWidth = sizeof(T) * elementCount;
		desc.StructureByteStride = sizeof(T);

		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;


		/*
		if (type == eStructuredBufferType::Read)
		{
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.Usage = initialData ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		}
		else if (type == eStructuredBufferType::Write)
		{
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		}
		else
		{
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		}
		*/
		D3D11_SUBRESOURCE_DATA subData{};
		subData.pSysMem = initialData;

		auto device = graphics->GetDevice();
		HRESULT hr = device->CreateBuffer(&desc, initialData ? &subData : nullptr, m_buffer.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("StructuredBuffer 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			assert(false && "StructuredBuffer 생성 실패!");
		}

		//if (type == eStructuredBufferType::Write || type == eStructuredBufferType::ReadWrite)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.NumElements = elementCount;

			hr = device->CreateUnorderedAccessView(m_buffer.Get(), &uavDesc, m_uav.GetAddressOf());
			if (FAILED(hr))
			{
				spdlog::error("StructuredBuffer UAV 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				assert(false && "StructuredBuffer UAV 생성 실패!");
			}
		}
		/*
		if (type == eStructuredBufferType::Read || type == eStructuredBufferType::ReadWrite)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = elementCount;

			hr = device->CreateShaderResourceView(m_buffer.Get(), &srvDesc, m_srv.GetAddressOf());
			if (FAILED(hr))
			{
				spdlog::error("StructuredBuffer SRV 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				assert(false && "StructuredBuffer SRV 생성 실패!");
			}
		}*/
	}
}