#pragma once
#include <string>
#include <array>
#include <unordered_map>
#include <d3d11.h>
#include <memory>
#include <wrl/client.h>
#include <DirectXTex/DirectXTex.h>

#include <vector>

namespace Dive
{
	using TextureHandle = uint64_t;
	inline constexpr TextureHandle const INVALID_TEXTURE_HANDLE = uint64_t(-1);

	class Graphics;

	struct TextureInfo
	{
		std::wstring filepath;
		std::string name;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	};

	class TextureManager
	{
	public:
		static TextureManager& Get()
		{
			static TextureManager inst;
			return inst;
		}

		void Initialize(Graphics* graphics);
		void Shutdown();

		TextureHandle LoadTexture(const std::wstring& filepath, bool mips = true);
		TextureHandle LoadTexture(const std::string& filepath, bool mips = true);
		TextureHandle LoadCubemap(const std::wstring& filepath);

		void SetFaceData(uint32_t index, const void* pixels, size_t size);

		std::wstring GetTextureFilepath(TextureHandle handle) const;
		std::string GetTextureName(TextureHandle handle) const;
		ID3D11ShaderResourceView* GetTextureView(TextureHandle handle) const;
		
	private:
		TextureManager() = default;
		TextureManager(const TextureManager&) = delete;
		TextureManager(TextureManager&&) = default;
		~TextureManager() = default;

		TextureManager& operator=(const TextureManager&) = delete;
		TextureManager& operator=(TextureManager&&) = default;

		TextureHandle loadDDSTexture(const std::wstring& filepath, bool mips);
		TextureHandle loadTGATexture(const std::wstring& filepath, bool mips);
		TextureHandle loadWICTexture(const std::wstring& filepath, bool mips);
		TextureHandle createTexture(const std::wstring& name, DirectX::ScratchImage& scratchImage, DirectX::TexMetadata& metaData, bool mips);

	private:
		Graphics* m_graphics = nullptr;
		
		TextureHandle m_handle = INVALID_TEXTURE_HANDLE;
		//std::unordered_map<TextureHandle, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
		std::unordered_map<TextureHandle, TextureInfo> m_textures;
		std::unordered_map<std::wstring, TextureHandle> m_loaded;

		std::array<std::vector<uint8_t>, 6> m_faceData;
	};
}