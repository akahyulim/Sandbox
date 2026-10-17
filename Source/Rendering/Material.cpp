#include "pch.h"
#include "Material.h"
#include "Graphics/Graphics.h"
#include "Utilities/YamlHelper.h"

namespace Dive
{
	Material::Material()
	{
		for (uint32_t i = 0; i < m_maps.size(); ++i)
		{
			m_maps[i] = INVALID_TEXTURE_HANDLE;
		}
	}

	Material::~Material() = default;

	// 여기에서도 dir만 받는 게 어떨까 싶다.
	// fullpath용을 따로 두는것도 방법이다.
	bool Material::SaveToFile(const std::filesystem::path& filepath)
	{
		if (!IsDirty())
			return true;

		std::string target = filepath.string() + ".mat";
		std::ofstream fout(target, std::ios::binary);
		if (!fout.is_open())
		{
			spdlog::error("머티리얼 파일 생성 실패: {}", target);
			return false;
		}

		if (!Serialize(fout))
			return false;

		m_filepath = filepath;

		ClearDirty();
		return true;
	}

	// 여기에서 붙이지 말고...
	// 흠.. 전달받을 땐 역시 포멧을 제외하는 게 나을 거 같은데...
	bool Material::LoadFromFile(const std::filesystem::path& filepath)
	{
		std::string target = filepath.string() + ".mat";
		std::ifstream fin(target, std::ios::binary);
		if (!fin.is_open())
		{
			spdlog::error("머티리얼 파일 열기 실패: {}", target);
			return false;
		}

		if (!Deserialize(fin))
			return false;

		m_filepath = filepath;

		ClearDirty();
		return true;
	}

	bool Material::Serialize(std::ofstream& fout) const
	{
		if (!fout.good())
		{
			spdlog::error("쓰기 스트림 오류");
			return false;
		}

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << GetName();
		out << YAML::Key << "Transparent" << YAML::Value << m_isTransparent;
		out << YAML::Key << "BaseColor" << YAML::Value << m_data.baseColor;
		out << YAML::Key << "EmissiveFactor" << YAML::Value << m_data.emissiveFactor;
		out << YAML::Key << "RoughnessFactor" << YAML::Value << m_data.roughnessFactor;
		out << YAML::Key << "Tiling" << YAML::Value << m_data.tiling;
		out << YAML::Key << "Offset" << YAML::Value << m_data.offset;
		out << YAML::Key << "MetalicFactor" << YAML::Value << m_data.metallicFactor;
		out << YAML::Key << "HeightScale" << YAML::Value << m_data.heightScale;

		out << YAML::Key << "Maps" << YAML::BeginMap;
		for (size_t i = 0; i < static_cast<size_t>(eMapType::Count); ++i)
		{
			if (!m_mapPaths[i].empty())
			{
				out << YAML::Key << GetMapTypeName(static_cast<eMapType>(i))
					<< YAML::Value << m_mapPaths[i];
			}
		}
		out << YAML::EndMap;

		out << YAML::EndMap;

		fout << out.c_str();

		return true;
	}

	bool Material::Deserialize(std::ifstream& fin)
	{
		if (!fin.good())
		{
			spdlog::error("읽기 스트림 오류");
			return false;
		}

		try
		{
			// 1. 파일 스트림으로부터 YAML 노드 로드
			YAML::Node root = YAML::Load(fin);
			if (!root.IsMap())
			{
				spdlog::error("잘못된 YAML 포맷입니다.");
				return false;
			}

			// 2. 기본 정보 복원
			if (root["Name"])
				SetName(root["Name"].as<std::string>());

			if (root["Transparent"])
				m_isTransparent = root["Transparent"].as<bool>();

			// convert 덕분에 한 줄로 깔끔하게 역직렬화됩니다!
			if (root["BaseColor"])
				m_data.baseColor = root["BaseColor"].as<DirectX::XMFLOAT4>();

			if (root["EmissiveFactor"])
				m_data.emissiveFactor = root["EmissiveFactor"].as<DirectX::XMFLOAT3>();

			if (root["RoughnessFactor"])
				m_data.roughnessFactor = root["RoughnessFactor"].as<float>();

			if (root["Tiling"])
				m_data.tiling = root["Tiling"].as<DirectX::XMFLOAT2>();

			if (root["Offset"])
				m_data.offset = root["Offset"].as<DirectX::XMFLOAT2>();

			if (root["MetalicFactor"])
				m_data.metallicFactor = root["MetalicFactor"].as<float>();

			if (root["HeightScale"])
				m_data.heightScale = root["HeightScale"].as<float>();

			// 3. 맵 경로 복원 및 텍스처 로드 연동
			if (root["Maps"] && root["Maps"].IsMap())
			{
				YAML::Node mapsNode = root["Maps"];
				for (auto it = mapsNode.begin(); it != mapsNode.end(); ++it)
				{
					std::string typeStr = it->first.as<std::string>();
					std::string path = it->second.as<std::string>();

					eMapType type = GetMapTypeFromString(typeStr);
					if (type != eMapType::Count && !path.empty())
					{
						// SetMap을 호출하여 경로 저장 및 텍스처 핸들 로드 동시 처리
						SetMap(type, path);
					}
				}
			}
		}
		catch (const YAML::Exception& e)
		{
			spdlog::error("YAML 파싱 실패: {}", e.what());
			return false;
		}

		return true;
	}

	ID3D11ShaderResourceView* Material::GetMap(eMapType type) const
	{
		if(type >= eMapType::Count)
			return nullptr;

		return TextureManager::Get().GetTextureView(m_maps[static_cast<size_t>(type)]);
	}

	void Material::SetMap(eMapType type, const std::string& path)
	{
		size_t slot = static_cast<size_t>(type);

		m_maps[slot] =  TextureManager::Get().LoadTexture(path);
		m_mapPaths[slot] = path;

		if (m_maps[slot] != INVALID_TEXTURE_HANDLE)
		{
			m_data.flags |= (1U << slot);

			if (type == eMapType::ORM)
			{
				m_data.roughnessFactor = 1.0f;
				m_data.metallicFactor = 1.0f;
			}
			else
			{
				if (type == eMapType::Roughness)
				{
					m_data.roughnessFactor = 1.0f;
				}

				if (type == eMapType::Metallic)
				{
					m_data.metallicFactor = 1.0f;
				}
			}
		}
		else
		{
			m_data.flags &= ~(1U << slot);

			if (type == eMapType::ORM)
			{
				m_data.roughnessFactor = 0.0f;
				m_data.metallicFactor = 0.0f;
			}
			else
			{

				if (type == eMapType::Roughness)
				{
					m_data.roughnessFactor = 0.0f;
				}

				if (type == eMapType::Metallic)
				{
					m_data.metallicFactor = 0.0f;
				}
			}
		}

		MarkDirty();
	}

	void Material::SetMap(eMapType type, TextureHandle handle)
	{
		size_t slot = static_cast<size_t>(type);
	
		if (nullptr == TextureManager::Get().GetTextureView(handle))
		{
			m_maps[slot] = INVALID_TEXTURE_HANDLE;
			m_mapPaths[slot] = "";
			m_data.flags &= ~(1U << slot);

			if (type == eMapType::ORM)
			{
				m_data.roughnessFactor = 0.0f;
				m_data.metallicFactor = 0.0f;
			}
			else
			{
				if (type == eMapType::Roughness)
				{
					m_data.roughnessFactor = 0.0f;
				}

				if (type == eMapType::Metallic)
				{
					m_data.metallicFactor = 0.0f;
				}
			}
		}
		else
		{
			m_maps[slot] = handle;
			m_mapPaths[slot] = TextureManager::Get().GetTextureFilepath(handle);
			m_data.flags |= (1U << slot);

			if (type == eMapType::ORM)
			{
				m_data.roughnessFactor = 1.0f;
				m_data.metallicFactor = 1.0f;
			}
			else
			{
				if (type == eMapType::Roughness)
				{
					m_data.roughnessFactor = 1.0f;
				}

				if (type == eMapType::Metallic)
				{
					m_data.metallicFactor = 1.0f;
				}
			}
		}

		MarkDirty();
	}

	bool Material::HasMap(eMapType type) const
	{
		size_t slot = static_cast<size_t>(type);
		return m_maps[slot] != INVALID_TEXTURE_HANDLE;
	}

	void Material::SetBaseColor(const DirectX::XMFLOAT4& color)
	{
		auto current = m_data.baseColor;
		if (current.x != color.x || current.y != color.y ||
			current.z != color.z || current.w != color.w)
		{
			m_data.baseColor = color;
			MarkDirty();
		}
	}

	void Material::SetBaseColor(float r, float g, float b, float a)
	{
		SetBaseColor(DirectX::XMFLOAT4(r, g, b, a));
	}

	void Material::SetEmissiveFactor(const DirectX::XMFLOAT3& factor)
	{
		auto current = m_data.emissiveFactor;
		if (current.x != factor.x || current.y != factor.y || current.z != factor.z)
		{
			m_data.emissiveFactor = factor;
			MarkDirty();
		}
	}

	void Material::SetEmissiveFactor(float r, float g, float b)
	{
		SetEmissiveFactor(DirectX::XMFLOAT3(r, g, b));
	}

	void Material::SetRoughnessFactor(float factor)
	{
		if (m_data.roughnessFactor != factor)
		{
			m_data.roughnessFactor = factor;
			MarkDirty();
		}
	}

	void Material::SetTiling(const DirectX::XMFLOAT2& tiling)
	{
		auto current = m_data.tiling;
		if (current.x != tiling.x || current.y != tiling.y)
		{
			m_data.tiling = tiling;
			MarkDirty();
		}
	}

	void Material::SetTiling(float x, float y)
	{
		SetTiling(DirectX::XMFLOAT2(x, y));
	}

	void Material::SetHeightScale(float scale)
	{
		if (scale != m_data.heightScale)
		{
			m_data.heightScale = scale;
			MarkDirty();
		}
	}

	void Material::SetOffset(const DirectX::XMFLOAT2& offset)
	{
		auto current = m_data.offset;
		if (current.x != offset.x || current.y != offset.y)
		{
			m_data.offset = offset;
			MarkDirty();
		}
	}

	void Material::SetOffset(float x, float y)
	{
		SetOffset(DirectX::XMFLOAT2(x, y));
	}

	void Material::SetMetallicFactor(float factor)
	{
		if (m_data.metallicFactor != factor)
		{
			m_data.metallicFactor = factor;
			MarkDirty();
		}
	}

	void Material::SetTransparent(bool transparent)
	{
		if (transparent != m_isTransparent)
		{
			m_isTransparent = transparent;
			MarkDirty();
		}
	}

	void Material::Bind(Graphics* graphics)
	{
		if (!m_cbuffer)
		{
			m_cbuffer = std::make_unique<ConstantBuffer<MaterialData>>(graphics);
		}

		for(uint32_t i = 0; i < m_maps.size(); ++i)
		{
			auto handle = m_maps[i];
			if (handle != INVALID_TEXTURE_HANDLE)
			{
				auto srv = TextureManager::Get().GetTextureView(handle);

				if (srv)
				{
					graphics->SetShaderResourceView(eShaderStage::DS, i, &srv);
					graphics->SetShaderResourceView(eShaderStage::PS, i, &srv);
				}
			}
		}

		m_cbuffer->Update(graphics, m_data);

		m_cbuffer->Bind(graphics, eShaderStage::DS, static_cast<uint32_t>(eConstantBuffer::Material));
		m_cbuffer->Bind(graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Material));
	}

	const char* Material::GetMapTypeName(eMapType type)
	{
		switch (type)
		{
		case eMapType::Albedo:       return "Albedo";
		case eMapType::Normal:       return "Normal";
		case eMapType::ORM:          return "ORM";
		case eMapType::Displacement: return "Displacement";
		case eMapType::Roughness:    return "Roughness";
		case eMapType::Metallic:     return "Metallic";
		default:                     return "Unknown";
		}
	}

	eMapType Material::GetMapTypeFromString(const std::string& name)
	{
		if (name == "Albedo")       return eMapType::Albedo;
		if (name == "Normal")       return eMapType::Normal;
		if (name == "ORM")          return eMapType::ORM;
		if (name == "Displacement") return eMapType::Displacement;
		if (name == "Roughness")    return eMapType::Roughness;
		if (name == "Metallic")     return eMapType::Metallic;

		return eMapType::Count;
	}
}
