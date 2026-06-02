#include "stdafx.h"
#include "Material.h"
#include "API/Graphics.h"
#include "API/Resources.h"
#include "API/Shaders.h"
#include "Resource/Texture2D.h"
#include "Shader/ShaderProgram.h"
#include "Graphics/GraphicsDevice.h"

namespace Dive
{
    Material::Material()
    {
        // Importer에서 GraphicsDevice가 초기화되지 않은 상태를 유지하기 위한 임시방편
        //if (GraphicsDevice::GetDevice())
        {
         //   m_cbMaterialPS = std::make_unique<ConstantBuffer>(static_cast<uint32_t>(sizeof(MaterialParams)));
          //  if (!m_cbMaterialPS) DV_LOG(Material, eLogLevel::Err, "[::Material] cbMaterialPS 생성 실패");
        }

        m_mapTextures.fill({});
        
        SetName("Legacy");

        //if(GraphicsDevice::GetDevice())
         //   m_shaderProgram = ShaderManager::GetProgram("Legacy");
    }

    Material::~Material()
    {
        DV_LOG(Material, eLogLevel::Info, "소멸: {}", GetName());
    }

    bool Material::Create(GraphicsDevice* device, std::shared_ptr<IResourceDesc> desc)
    {
        auto materialDesc = std::static_pointer_cast<MaterialDesc>(desc);
        if (!materialDesc)
            return false;

        m_params = materialDesc->params;
        m_mapPaths = materialDesc->mapPaths;
        m_shaderName = materialDesc->shaderName;

        m_cbMaterialPS = device->CreateConstantBuffer<MaterialParams>();
        if (!m_cbMaterialPS)
            return false;

        MarkDirty();

        return true;
    }

    void Material::Bind(GraphicsDevice* device)
    {
        if (!m_shaderProgram)
            return;
        m_shaderProgram->Bind(device);

        if (m_cbMaterialPS)
        {
            if(m_isDirty)
                device->UpdateConstantBuffer(m_cbMaterialPS.get(), &m_params);
            device->BindPSConstantBuffer(eCBufferSlotPS::Material, m_cbMaterialPS.get());
        }

        // 텍스쳐는 일단 생략
        /*
        if (m_mapTextures[static_cast<size_t>(eMapType::Diffuse)])
            m_mapTextures[static_cast<size_t>(eMapType::Diffuse)]->Bind(eShaderResourceSlot::Diffuse);
        if (m_mapTextures[static_cast<size_t>(eMapType::Normal)])
            m_mapTextures[static_cast<size_t>(eMapType::Normal)]->Bind(eShaderResourceSlot::NormalMap);
         */

        ClearDirty();
    }

    std::shared_ptr<Texture2D> Material::GetMap(eMapType type)
    {
        if (type == eMapType::Count)
        {
            DV_LOG(Material, eLogLevel::Err, "[::GetMap] 잘못된 맵 타입");
            return {};
        }

		return m_mapTextures[static_cast<size_t>(type)];
    }

    void Material::SetMap(eMapType type, std::shared_ptr<Texture2D> texture)
    {
        if (type == eMapType::Count)
        {
            DV_LOG(Material, eLogLevel::Err, "[::SetMap] 잘못된 맵 타입");
            return;
        }

        auto& slot = m_mapTextures[static_cast<size_t>(type)];
        if (slot != texture)
        {
            slot = texture; // 빈 weak_ptr도 대입 가능
            m_mapPaths[static_cast<size_t>(type)] = texture ? texture->GetFilepath() : "";
            MarkDirty();
        }

        switch (type)
        {
        case eMapType::Diffuse:
            m_params.flags |= slot ? (1U << 0) : 0;
            break;
        case eMapType::Normal:
            m_params.flags |= slot ? (1U << 1) : 0;
            break;
        default:
            break;
        }
    }


    void Material::SetMap(eMapType type, const std::string& texturePath)
    {
        SetMap(type, Resources::Load<Texture2D>(texturePath));
    }

    void Material::SetDiffuseColor(const DirectX::XMFLOAT4& color)
    {
        if (m_params.diffuseColor.x != color.x ||
            m_params.diffuseColor.y != color.y ||
            m_params.diffuseColor.z != color.z ||
            m_params.diffuseColor.w != color.w)
        {
            m_params.diffuseColor = color;
            SetHasAlpha(color.w < 1.0f);
            MarkDirty();
        }
    }

    void Material::SetDiffuseColor(float r, float g, float b, float a)
    {
        SetDiffuseColor(DirectX::XMFLOAT4(r, g, b, a));
    }

    void Material::SetTiling(float x, float y)
    {
        if (m_params.tiling.x == x && m_params.tiling.y == y)
            return;

        m_params.tiling = { x, y };
        MarkDirty();
    }

    void Material::SetOffset(float x, float y)
    {
        if (m_params.offset.x == x && m_params.offset.y == y)
            return;

        m_params.offset = { x, y };
        MarkDirty();
    }

    void Material::SetHasAlpha(bool hasAlpha)
    {
        if (m_hasAlpha != hasAlpha)
            m_hasAlpha = hasAlpha;

        if (hasAlpha)
            m_params.renderingMode = static_cast<uint32_t>(eRenderingMode::Transparent);

        MarkDirty();
    }

    std::string Material::GetShaderProgramName() const
    {
        if(!m_shaderProgram)
            return std::string();

        return m_shaderProgram->GetName();
    }

    void Material::SetShaderProgramByName(const std::string& name)
    {
        if (m_shaderProgram != Shaders::GetProgram(name))
        {
            m_shaderProgram = Shaders::GetProgram(name);
            MarkDirty();
        }
    }

    bool Material::IsTransparent() const
    {
        return m_params.renderingMode != static_cast<uint32_t>(eRenderingMode::Opqaue);
    }

    bool Material::Serialize(std::ofstream& fout) const
    {
        if (!fout.good()) return false;

        YAML::Emitter out;
        out << YAML::BeginMap;

        // 1. 메타데이터
        out << YAML::Key << "Name" << YAML::Value << GetName();
        out << YAML::Key << "Shader" << YAML::Value << (m_shaderProgram ? m_shaderProgram->GetName() : "DefaultShader");
        out << YAML::Key << "RenderingMode" << YAML::Value << static_cast<uint32_t>(m_params.renderingMode);

        // 2. 텍스처 경로 (리스트 형태로 관리하면 확장성이 좋습니다)
        out << YAML::Key << "Maps";
        out << YAML::BeginMap;
        out << YAML::Key << "Diffuse" << YAML::Value << m_mapPaths[static_cast<size_t>(eMapType::Diffuse)].generic_string();
        out << YAML::Key << "Normal" << YAML::Value << m_mapPaths[static_cast<size_t>(eMapType::Normal)].generic_string();
        out << YAML::EndMap;

        // 3. 수치 데이터 (MaterialParams 구조체 내용)
        out << YAML::Key << "Properties";
        out << YAML::BeginMap;
        out << YAML::Key << "DiffuseColor" << YAML::Value << m_params.diffuseColor;
        out << YAML::Key << "Tiling" << YAML::Value << m_params.tiling;
        out << YAML::Key << "Offset" << YAML::Value << m_params.offset;
        out << YAML::Key << "Flags" << YAML::Value << m_params.flags;
        out << YAML::EndMap;

        out << YAML::EndMap;
        fout << out.c_str();

        return true;
    }

    // 데이터 로드와 버퍼 생성을 동시에 하고 있다.
    // 이를 분리하기 위해 Resource::Create가 있는 거다.
    bool Material::Deserialize(std::ifstream& fin)
    {
        if (!fin.good())
        {
            DV_LOG(Material, eLogLevel::Err, "[::Deserizlie] 읽기 스트림 오류");
            return false;
        }

        YAML::Node data;
        try
        {
            data = YAML::Load(fin);
        }
        catch (const YAML::Exception& e)
        {
            DV_LOG(Material, eLogLevel::Err, "[::Deserialize] YAML 처리 실패: {}", e.what());
            return false;
        }

        // 1. 수치 데이터 복구
        if (auto props = data["Properties"]) {
            m_params.diffuseColor = props["DiffuseColor"].as<DirectX::XMFLOAT4>();
            m_params.tiling = props["Tiling"].as<DirectX::XMFLOAT2>();
            m_params.offset = props["Offset"].as<DirectX::XMFLOAT2>();
            m_params.flags = props["Flags"].as<uint32_t>();
        }

        // 2. 경로 문자열만 보관 (실제 로드는 하지 않음)
        if (auto maps = data["Maps"]) {
            m_mapPaths[static_cast<size_t>(eMapType::Diffuse)] = maps["Diffuse"].as<std::string>();
            m_mapPaths[static_cast<size_t>(eMapType::Normal)] = maps["Normal"].as<std::string>();
        }

        // 3. 셰이더 이름 보관
        if (data["Shader"]) m_shaderName = data["Shader"].as<std::string>();

        return true;
    }
}
