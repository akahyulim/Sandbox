#pragma once
#include <filesystem>

namespace Dive
{
	class ResourceDataLoader
	{
	public:
		struct TextureResult
		{
			std::unique_ptr<DirectX::ScratchImage> scratchImage;
			std::unique_ptr<DirectX::TexMetadata> metaData;
		};

		static std::unique_ptr<TextureResult> LoadTextureData(const std::filesystem::path& filepath);
	};
}