#include "pch.h"
#include "ShaderManager.h"
#include "Graphics/Graphics.h"

namespace Dive
{
	namespace
	{
		Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::filesystem::path& filepath, eShaderStage stage)
		{
			Microsoft::WRL::ComPtr<ID3DBlob> blob = nullptr;
			Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

			DWORD compileFlags = 0;
#if _DEBUG
			compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL;
#else
			compileFlags |= D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

			std::string entryPoint, target;
			if (stage == eShaderStage::VS)
			{
				entryPoint = "MainVS";
				target = "vs_5_0";
			}
			else if (stage == eShaderStage::PS)
			{
				entryPoint = "MainPS";
				target = "ps_5_0";
			}
			else
			{
				spdlog::error("지원하지 않는 셰이더 스테이지입니다.");
				return nullptr;
			}

			HRESULT hr = D3DCompileFromFile(
				filepath.wstring().c_str(),
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				entryPoint.c_str(),
				target.c_str(),
				compileFlags,
				0,
				blob.GetAddressOf(),
				errorBlob.GetAddressOf()
			);

			if (FAILED(hr))
			{
				if (errorBlob != nullptr)
				{
					spdlog::error("셰이더 컴파일 실패 [{}]: {}",
						filepath.string(),
						(char*)errorBlob->GetBufferPointer());
				}
				return nullptr;
			}

			return blob;
		}
	}

	ShaderManager::~ShaderManager() = default;

	// adria는 initialize에서 CompileAllSahders를 호출하고 내부에서 CrateAllPrograms를 호출한다.
	bool ShaderManager::Initialize(Graphics* graphics)
	{
		assert(graphics);

		// vertex shader and input layout
		if (!createVertexShaderAndInputLayout(graphics, "Source/Shaders/UnlitVS.hlsl", eInputLayout::Unlit))
		{
			spdlog::error("UnlitVS 생성 실패");
			return false;
		}
		if(!createVertexShaderAndInputLayout(graphics, "Source/Shaders/LitVS.hlsl", eInputLayout::Lit))
		{
			spdlog::error("LitVS 생성 실패");
			return false;
		}
		if (!createVertexShaderAndInputLayout(graphics, "Source/Shaders/Skybox.hlsl", eInputLayout::Position))
		{
			spdlog::error("SkyboVS 생성 실패");
			return false;
		}
		if (!createVertexShaderAndInputLayout(graphics, "Source/Shaders/ResolveSceneVS.hlsl", eInputLayout::None))
		{
			spdlog::error("ResolveSceneVS 생성 실패");
			return false;
		}
		

		// pixel shader
		if(!createPixelShader(graphics, "Source/Shaders/UnlitPS.hlsl"))
		{
			spdlog::error("UnlitPS 생성 실패");
			return false;
		}
		if(!createPixelShader(graphics, "Source/Shaders/LegacyPS.hlsl"))
		{
			spdlog::error("LegacyPS 생성 실패");
			return false;
		}
		if (!createPixelShader(graphics, "Source/Shaders/PbsPS.hlsl"))
		{
			spdlog::error("PbsPS 생성 실패");
			return false;
		}
		if (!createPixelShader(graphics, "Source/Shaders/Skybox.hlsl"))
		{
			spdlog::error("SkyboxPS 생성 실패");
			return false;
		}
		if (!createPixelShader(graphics, "Source/Shaders/ResolveScenePS.hlsl"))
		{
			spdlog::error("ResolveScenePS 생성 실패");
			return false;
		}
		

		// shader program
		if(!createShaderProgram("UnlitVS", "UnlitPS", eShaderPrograms::Unlit))
		{
			spdlog::error("Unlit ShaderProgram 생성 실패");
			return false;
		}
		if(!createShaderProgram("LitVS", "LegacyPS", eShaderPrograms::LegacyLit))
		{
			spdlog::error("Legacy ShaderProgram 생성 실패");
			return false;
		}
		if (!createShaderProgram("LitVS", "PbsPS", eShaderPrograms::PbsLit))
		{
			spdlog::error("DefaultLit ShaderProgram 생성 실패");
			return false;
		}
		//if (!createShaderProgram("LitVS", "UnlitPS", "DefaultUnlit"))
		//{
		//	spdlog::error("Legacy ShaderProgram 생성 실패");
		//	return false;
		//}
		// 하나의 hlsl파일에 vs, ps를 모두 구현하면 이렇게 동일한 이름으로 저장된다.
		// enum class로 미리 선언해놓는 것도 하나의 방법이다.
		if (!createShaderProgram("Skybox", "Skybox", eShaderPrograms::Skybox))
		{
			spdlog::error("Skybox ShaderProgram 생성 실패");
			return false;
		}
		if (!createShaderProgram("ResolveSceneVS", "ResolveScenePS", eShaderPrograms::Resolve))
		{
			spdlog::error("ResloveScene ShaderProgram 생성 실패");
			return false;
		}

		spdlog::info("ShaderManager 초기화 완료");

		return true;
	}

	ShaderProgram* ShaderManager::GetShaderProgram(eShaderPrograms sp)
	{
		auto it = m_shaderPrograms.find(sp);
		if (it != m_shaderPrograms.end())
			return it->second.get();

		return nullptr;
	}

	bool ShaderManager::createVertexShaderAndInputLayout(Graphics* graphics, const std::filesystem::path& path, eInputLayout type)
	{
		std::string shaderName = path.stem().string();
		if (m_vss.find(shaderName) != m_vss.end())
		{
			spdlog::warn("이미 생성 및 저장된 셰이더");
			return false;
		}

		auto blob = CompileShader(path, eShaderStage::VS);
		if (!blob)
			return false;

		m_vss[shaderName] = std::make_unique<VertexShader>(graphics, blob.Get());

		if (eInputLayout::None != type)
			m_ils[shaderName] = std::make_unique<InputLayout>(graphics, type, blob.Get());

		return true;
	}

	bool ShaderManager::createPixelShader(Graphics* graphics, const std::filesystem::path& path)
	{
		std::string shaderName = path.stem().string();
		if (m_pss.find(shaderName) != m_pss.end())
		{
			spdlog::warn("이미 생성 및 저장된 셰이더");
			return false;
		}

		auto blob = CompileShader(path, eShaderStage::PS);
		if (!blob)
			return false;

		m_pss[shaderName] = std::make_unique<PixelShader>(graphics, blob.Get());

		return true;
	}

	bool ShaderManager::createShaderProgram(const std::string& vsName, const std::string& psName, eShaderPrograms sp)
	{
		VertexShader* vs = nullptr;
		{
			auto it = m_vss.find(vsName);
			if (it != m_vss.end())
				vs = it->second.get();
			else
				return false;
		}

		PixelShader* ps = nullptr;
		{
			auto it = m_pss.find(psName);
			if (it != m_pss.end())
				ps = it->second.get();
			else
				return false;
		}

		InputLayout* il = nullptr;
		{
			auto it = m_ils.find(vsName);
			if (it != m_ils.end())
				il = it->second.get();
		}

		m_shaderPrograms[sp] = std::make_unique<ShaderProgram>(vs, ps, il);

		return true;
	}
}