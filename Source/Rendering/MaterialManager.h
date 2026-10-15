#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>

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

        void SaveToFiles();
        Material* LoadFromFile(const std::filesystem::path& filepath);
        
        Material* CreateMaterial(const std::string& name);

        Material* GeStandard() const;
        Material* GetMaterial(const std::string& name) const;
        const std::vector<Material*>& GetAllMaterials() const { return m_materialList; }

        std::string GetDirectory() const { return m_directory.string(); }
        void SetDirectory(const std::filesystem::path& dir) { m_directory = dir; }

    private:
        MaterialManager() = default;
        ~MaterialManager() = default;

    private:
        std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;
        std::vector<Material*> m_materialList;
        std::filesystem::path m_directory;
    };
}