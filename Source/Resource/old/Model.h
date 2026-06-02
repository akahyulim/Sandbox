#pragma once
#include "Resource.h"
#include <DirectXMath.h>

namespace Dive
{
	class Mesh;
	class Material;

	struct ModelNode
	{
		std::string name;
		int parentIndex = -1;
		std::vector<uint32_t> children;

		DirectX::XMFLOAT4X4 localTransform;

		struct MeshInstance
		{
			std::string meshPath;
			std::string materialPath;

			std::shared_ptr<Mesh> mesh;
			std::shared_ptr<Material> material;
		};
		std::vector<MeshInstance> meshInstances;

		ModelNode()
		{
			DirectX::XMStoreFloat4x4(&localTransform, DirectX::XMMatrixIdentity());
		}
	};

	class Model : public Resource
	{
	public:
		Model();
		~Model() override;

		virtual bool Create() override;
		virtual void Release() override;

		//bool SaveToFile(const std::filesystem::path& filepath) override;
		//bool LoadFromFile(const std::filesystem::path& filepath) override;

		const std::vector<ModelNode>& GetNodes() const { return m_nodes; }
		void AddNode(const ModelNode& node) { m_nodes.push_back(node); }
			
		eResourceType GetType() const override { return eResourceType::Model; }
		static constexpr eResourceType StaticType() { return eResourceType::Model; };

	protected:
		virtual bool Serialize(std::ofstream& fout) const override;
		virtual bool Deserialize(std::ifstream& fin) override;

	private:
		std::vector<ModelNode> m_nodes;
	};
}