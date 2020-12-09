#pragma once

namespace shadow
{
    class ResourceManager
    {
    public:
        ~ResourceManager() = default;
        ResourceManager(ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        ResourceManager& operator=(ResourceManager&) = delete;
        ResourceManager& operator=(ResourceManager&&) = delete;
        static ResourceManager& getInstance();
    private:
        ResourceManager() = default;
    };
}
