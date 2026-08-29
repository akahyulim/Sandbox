#pragma once
#include <memory>
#include <unordered_map>
#include <string>

#include "Material.h"

namespace Dive
{
    class MaterialManager
    {
    public:
        static MaterialManager& Get()
        {
            static MaterialManager instance;
            return instance;
        }

        void Initialize();
        void Shutdown() { m_materials.clear(); }

        Material* LoadFromFile(const std::string& path);
        Material* CreateMaterial(const std::string& name);

        Material* GetMaterial(const std::string& name) const;

    private:
        MaterialManager() = default;
        ~MaterialManager() = default;

    private:
        std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;
    };
}