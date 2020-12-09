#include "ResourceManager.h"
shadow::ResourceManager& shadow::ResourceManager::getInstance()
{
    static ResourceManager resourceManager{};
    return resourceManager;
}
