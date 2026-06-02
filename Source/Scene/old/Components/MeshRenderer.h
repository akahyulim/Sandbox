#pragma once
#include <DirectXMath.h>
#include <memory>
#include <yaml-cpp/yaml.h>
#include "Component.h"
#include "../Preset.h"
#include "Common/Object.h"
#include "Graphics/ConstantBuffer.h"
#include "Rendering/Frustum.h"
#include "Resource/StaticMesh.h"

namespace Dive
{
	//class StaticMesh;
	class Material;
	class ConstantBuffer;
	class Camera;
	struct Ray;

	struct alignas(16) ObjectData
	{
		DirectX::XMFLOAT4X4 worldMatrix;
	};

	class MeshRenderer : public Component
	{
	public:
		MeshRenderer(GameObject* owner, uint64_t id = AUTO_ID);
		~MeshRenderer() override = default;

		bool IsEqualTo(const Component* other) const override;

		std::unique_ptr<Component> Clone(GameObject* owner) const override;

		void Serialize(YAML::Emitter& out) override;
		void Deserialize(const YAML::Node& node) override;

		void Update() override;

		virtual void Render();

		bool IsVisible(const Frustum& frustum) const;
		bool Intersects(const Ray& ray, float* outDistance, DirectX::XMFLOAT3* outPoint, DirectX::XMFLOAT3* outNormal) const;
		
		// 여전히 Mesh쪽에서 관리하는 게 나아보인다.
		bool IsPresetTypeMesh() const { return m_presetType != ePresetType::None; }
		ePresetType GetPresetType() const { return m_presetType; }
		void SetPresetType(ePresetType type) { m_presetType = type; }
 
		std::shared_ptr<StaticMesh> GetStaticMesh() const { return m_staticMesh; }
		void SetStaticMesh(std::shared_ptr<StaticMesh> staticMesh);

		std::shared_ptr<Material> GetMaterial() const { return m_material; }
		void SetMaterial(std::shared_ptr<Material> material);
		void CloneMaterial();

		Bounds GetBounds() const;

		static constexpr eComponentType GetComponentType() { return eComponentType::MeshRenderer; }

	private:
		ePresetType m_presetType = ePresetType::None;
		std::shared_ptr<StaticMesh> m_staticMesh;
		std::shared_ptr<Material> m_material;

		std::unique_ptr<ConstantBuffer> m_cbObjectVS;
	};
}
