#pragma once

#include "DirectionalLight.h"
#include "SpotLight.h"
#include "UniformBufferObject.h"

namespace shadow
{
    struct Lights final
    {
        DirectionalLightData dirLightData{};
        SpotLightData spotLightData{};
        glm::vec3 paddingL{};
        float ambient{};
    };

    class UboLights final : UniformBufferObject<Lights>
    {
    public:
        UboLights(std::shared_ptr<DirectionalLight> directionalLight, std::shared_ptr<SpotLight> spotLight);
        void update();
        std::shared_ptr<DirectionalLight> getDirectionalLight() const;
        std::shared_ptr<SpotLight> getSpotLight() const;
        void setAmbient(float ambient);
    private:
        std::shared_ptr<DirectionalLight> directionalLight{};
        std::shared_ptr<SpotLight> spotLight{};
    };
}
