#pragma once

#include "ShadowLog.h"
#include "DirectionalLight.h"
#include "SpotLight.h"

#include "glad/glad.h"

namespace shadow
{
    class LightManager final
    {
    public:
        ~LightManager();
        static LightManager& getInstance();
        bool initialize();
        std::vector<std::shared_ptr<DirectionalLight>> getDirectionalLights() const;
        std::vector<std::shared_ptr<SpotLight>> getSpotLights() const;
        void addDirectionalLight(std::shared_ptr<DirectionalLight> directionalLight);
        void addSpotLight(std::shared_ptr<SpotLight> spotLight);
        void updateLights();
    private:
        LightManager() = default;
        static GLuint createSsbo(GLuint binding);
        static void rebuildSsbo(GLuint ssbo, void* data, GLsizeiptr size);
        static void updateSsbo(GLuint ssbo, void* data, GLsizeiptr size, GLintptr offset);
        std::vector<std::shared_ptr<DirectionalLight>> directionalLights{};
        std::vector<std::shared_ptr<SpotLight>> spotLights{};
        GLuint dirSsbo{}, spotSsbo{};
    };
}
