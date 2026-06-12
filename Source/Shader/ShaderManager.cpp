#include "pch.h"
#include "ShaderManager.h"
#include "Shader.h"
#include "InputLayout.h"
#include "ShaderProgram.h"
#include "Graphics/Graphics.h"

namespace Dive
{
	namespace {
		Microsoft::WRL::ComPtr<ID3DBlob> LoadShaderFile(const std::filesystem::path& path)
		{
			Microsoft::WRL::ComPtr<ID3DBlob> blob;

			if(FAILED(D3DReadFileToBlob(path.c_str(), blob.GetAddressOf())))
			{
				spdlog::error("셰이더 파일 로드 실패: {}", path.string());
				return nullptr;
			}

			return blob;
		}
	}

	ShaderManager::~ShaderManager() = default;

	bool ShaderManager::Initialize(Graphics* graphics)
	{
		assert(graphics);

		// vertex shader and input layout
		if (!createVertexShaderAndInputLayout(graphics, "Assets/Shaders/UnlitVS.cso", eInputLayout::Unlit))
		{
			spdlog::error("UnlitVS 생성 실패");
			return false;
		}
		if(!createVertexShaderAndInputLayout(graphics, "Assets/Shaders/LitVS.cso", eInputLayout::Lit))
		{
			spdlog::error("LitVS 생성 실패");
			return false;
		}
		

		// pixel shader
		if(!createPixelShader(graphics, "Assets/Shaders/UnlitPS.cso"))
		{
			spdlog::error("UnlitPS 생성 실패");
			return false;
		}
		if(!createPixelShader(graphics, "Assets/Shaders/LegacyPS.cso"))
		{
			spdlog::error("LegacyPS 생성 실패");
			return false;
		}
		if (!createPixelShader(graphics, "Assets/Shaders/PbsPS.cso"))
		{
			spdlog::error("PbsPS 생성 실패");
			return false;
		}
		

		// shader program
		if(!createShaderProgram("UnlitVS", "UnlitPS", "Unlit"))
		{
			spdlog::error("Unlit ShaderProgram 생성 실패");
			return false;
		}
		if(!createShaderProgram("LitVS", "LegacyPS", "LegacyLit"))
		{
			spdlog::error("Legacy ShaderProgram 생성 실패");
			return false;
		}
		if (!createShaderProgram("LitVS", "PbsPS", "DefaultLit"))
		{
			spdlog::error("DefaultLit ShaderProgram 생성 실패");
			return false;
		}
		if (!createShaderProgram("LitVS", "UnlitPS", "DefaultUnlit"))
		{
			spdlog::error("Legacy ShaderProgram 생성 실패");
			return false;
		}

		spdlog::info("ShaderManager 초기화 완료");

		return true;
	}

	std::shared_ptr<ShaderProgram> ShaderManager::GetProgram(const std::string& name)
	{
		auto it = m_shaderPrograms.find(name);
		if (it == m_shaderPrograms.end())
		{
			spdlog::warn("ShaderManaager::GetProgram - 존재하지 않는 셰이더 프로그램 요청: {}", name);
			return nullptr;
		}
		return it->second;
	}

	bool ShaderManager::createVertexShaderAndInputLayout(Graphics* graphics, const std::filesystem::path& path, eInputLayout type)
	{
		auto blob = LoadShaderFile(path);
		if (!blob)
			return false;

		const void* byteCode = blob->GetBufferPointer();
		size_t size = blob->GetBufferSize();

		auto vs = graphics->CreateVertexShader(byteCode, size);
		if (!vs)
			return false;

		std::string shaderName = path.stem().string();
		vs->SetName(shaderName);

		std::shared_ptr<InputLayout> il = nullptr;
		if (type != eInputLayout::None)
		{
			il = graphics->CreateInputLayout(type, byteCode, size);
			if (!il)
			{
				spdlog::error("{}용 InputLayout 생성 실패", shaderName);
				return false;
			}
		}

		m_vertexShaders[shaderName] = { std::move(vs), std::move(il) };
	
		return true;
	}

	bool ShaderManager::createPixelShader(Graphics* graphics, const std::filesystem::path& path)
	{
		auto blob = LoadShaderFile(path);
		if (!blob)
			return false;

		const void* byteCode = blob->GetBufferPointer();
		size_t size = blob->GetBufferSize();

		auto ps = graphics->CreatePixelShader(byteCode, size);
		if (!ps)
			return false;

		std::string shaderName = path.stem().string();
		ps->SetName(shaderName);

		m_pixelShaders[shaderName] = ps;

		return true;
	}

	bool ShaderManager::createShaderProgram(const std::string& vs, const std::string& ps, const std::string& name)
	{
		auto vm_it = m_vertexShaders.find(vs);
		if (vm_it == m_vertexShaders.end())
		{
			return false;
		}

		auto pm_it = m_pixelShaders.find(ps);
		if (pm_it == m_pixelShaders.end())
		{
			return false;
		}

		m_shaderPrograms[name] = std::make_shared<ShaderProgram>(vm_it->second.first, pm_it->second, vm_it->second.second, name);

		return true;
	}
}