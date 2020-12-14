#pragma once

#include "DirectedLight.h"

namespace shadow
{
    struct SpotLightData
    {
        glm::vec3 color{};
        float strength;
        glm::vec3 direction{};
        float innerCutOff{};
        glm::vec3 position{};
        float outerCutOff{};
    };

    class SpotLight final : public DirectedLight<SpotLightData>
    {
    public:
        glm::mat4 getLightSpaceMatrix() override;
        void setColor(glm::vec3& color) override;
        void setStrength(float strength) override;
        void setDirection(glm::vec3& direction) override;
        void setPosition(glm::vec3& position) override;
        void setInnerCutOff(float innerCutOff);
        void setOuterCutOff(float outerCutOff);
    private:
        glm::mat4 lightSpaceMatrix{};
    };
}
