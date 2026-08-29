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

		if (!createVertexShaderAndInputLayout(graphics, "Source/Shaders/Test.hlsl", eInputLayout::Lit))
		{
			spdlog::error("Test 생성 실패");
			return false;
		}
		if (!createVertexShaderAndInputLayout(graphics, "Source/Shaders/GBuffer.hlsl", eInputLayout::Lit))
		{
			spdlog::error("GBuffer VS 생성 실패");
			return false;
		}
		if (!createVertexShaderAndInputLayout(graphics, "Source/Shaders/DeferredLighting.hlsl", eInputLayout::None))
		{
			spdlog::error("DeferredLighting VS 생성 실패");
			return false;
		}
		if (!createVertexShaderAndInputLayout(graphics, "Source/Shaders/Skybox.hlsl", eInputLayout::Position))
		{
			spdlog::error("Skybox VS 생성 실패");
			return false;
		}
		if (!createVertexShaderAndInputLayout(graphics, "Source/Shaders/ResolveScene.hlsl", eInputLayout::None))
		{
			spdlog::error("ResolveScene VS 생성 실패");
			return false;
		}
		

		// pixel shader
		if (!createPixelShader(graphics, "Source/Shaders/Test.hlsl"))
		{
			spdlog::error("Test 생성 실패");
			return false;
		}
		if(!createPixelShader(graphics, "Source/Shaders/GBuffer.hlsl"))
		{
			spdlog::error("GBuffer PS 생성 실패");
			return false;
		}
		if (!createPixelShader(graphics, "Source/Shaders/DeferredLighting.hlsl"))
		{
			spdlog::error("DeferredLighting PS 생성 실패");
			return false;
		}
		if (!createPixelShader(graphics, "Source/Shaders/Skybox.hlsl"))
		{
			spdlog::error("Skybox PS 생성 실패");
			return false;
		}
		if (!createPixelShader(graphics, "Source/Shaders/ResolveScene.hlsl"))
		{
			spdlog::error("ResolveScene PS 생성 실패");
			return false;
		}
		

		// shader program
		if (!createShaderProgram("Test", "Test", eShaderPrograms::Test))
		{
			spdlog::error("Test 생성 실패");
			return false;
		}
		if(!createShaderProgram("GBuffer", "GBuffer", eShaderPrograms::GBuffer))
		{
			spdlog::error("GBuffer ShaderProgram 생성 실패");
			return false;
		}
		if (!createShaderProgram("DeferredLighting", "DeferredLighting", eShaderPrograms::DeferredLighting))
		{
			spdlog::error("DeferredLighting ShaderProgram 생성 실패");
			return false;
		}
		// 하나의 hlsl파일에 vs, ps를 모두 구현하면 이렇게 동일한 이름으로 저장된다.
		// enum class로 미리 선언해놓는 것도 하나의 방법이다.
		if (!createShaderProgram("Skybox", "Skybox", eShaderPrograms::Skybox))
		{
			spdlog::error("Skybox ShaderProgram 생성 실패");
			return false;
		}
		if (!createShaderProgram("ResolveScene", "ResolveScene", eShaderPrograms::Resolve))
		{
			spdlog::error("ResloveScene ShaderProgram 생성 실패");
			return false;
		}

		spdlog::info("ShaderManager 초기화 완료");

		return true;
	}

	void ShaderManager::Shutdown()
	{
		m_shaderPrograms.clear();
		m_ils.clear();
		m_pss.clear();
		m_vss.clear();
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