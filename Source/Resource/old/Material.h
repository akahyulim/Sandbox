#pragma once
#include <DirectXMath.h>
#include <memory>
#include <fstream>
#include <string>
#include "Resource.h"
//#include "Graphics/GraphicsDevice.h"
//#include "Graphics/ConstantBuffer.h"
//#include "Graphics/ShaderProgram.h"

#include "ResourceDesc.h"

namespace Dive
{
	class Texture;
	class Texture2D;
	class ShaderProgram;

	class Material : public Resource
	{
	public:
		Material();
		~Material() override;

		virtual bool Create() override;
		virtual void Release() override {};

		// 추후 이걸로 전부 교체
		bool Create(GraphicsDevice* device, std::shared_ptr<IResourceDesc> desc);
		
		// 근데 이건 Resource에 포함시키기 애매하다.
		// Texture류는 GraphicsDevice에 전달하는 게 낫다고 한다.
		void Bind(GraphicsDevice* device);

		std::shared_ptr<Texture2D> GetMap(eMapType type);
		void SetMap(eMapType type, std::shared_ptr<Texture2D> texture);
		void SetMap(eMapType type, const std::string& texturePath);
		
		std::filesystem::path GetMapPath(eMapType type) const { return m_mapPaths[static_cast<size_t>(type)]; }
		void SetMapPath(eMapType type, const std::filesystem::path& filepath) { m_mapPaths[static_cast<size_t>(type)] = filepath; }

		DirectX::XMFLOAT4 GetDiffuseColor() const { return m_params.diffuseColor; }
		void SetDiffuseColor(const DirectX::XMFLOAT4& color);
		void SetDiffuseColor(float r, float g, float b, float a);

		DirectX::XMFLOAT2 GetTiling() const { return m_params.tiling; }
		void SetTiling(float x, float y);

		DirectX::XMFLOAT2 GetOffset() const { return m_params.offset; }
		void SetOffset(float x, float y);

		bool HasAlpha() const { return m_hasAlpha; }
		void SetHasAlpha(bool hasAlpha);

		eRenderingMode GetRenderingMode() const { return static_cast<eRenderingMode>(m_params.renderingMode); }
		void SetRenderingMode(eRenderingMode mode) { m_params.renderingMode = static_cast<uint32_t>(mode); }

		const MaterialParams& GetData() { return m_params; }

		std::shared_ptr<ShaderProgram> GetShaderProgram() const { return m_shaderProgram; }
		void SetShaderProgram(std::shared_ptr<ShaderProgram> program) { m_shaderProgram = program; }

		std::string GetShaderName() const { return m_shaderName; }
		void SetShaderName(const std::string& shader) { m_shaderName = shader; }
		
		std::string GetShaderProgramName() const;
		void SetShaderProgramByName(const std::string& name);

		bool IsTransparent() const;

		eResourceType GetType() const override { return eResourceType::Material; }
		static constexpr eResourceType StaticType() { return eResourceType::Material; }

	protected:
		virtual bool Serialize(std::ofstream& fout) const override;
		virtual bool Deserialize(std::ifstream& fin) override;

	private:
		std::array<std::shared_ptr<Texture2D>, static_cast<size_t>(eMapType::Count)> m_mapTextures;
		std::array<std::filesystem::path, static_cast<size_t>(eMapType::Count)> m_mapPaths;

		MaterialParams m_params;

		bool m_hasAlpha = false;
		
		std::unique_ptr<ConstantBuffer> m_cbMaterialPS;

		std::string m_shaderName;
		std::shared_ptr<ShaderProgram> m_shaderProgram;
	};
}
