#pragma once

#include "DirectedLight.h"

namespace shadow
{
    struct DirectionalLightData
    {
        glm::vec3 color{ 0.0f, 0.0f, 0.0f };
        float strength{ 0.0f };
        glm::vec3 direction{ 0.0f, 0.0f, -1.0f };
    };

    class DirectionalLight final : public DirectedLight<DirectionalLightData>
    {
    public:
        DirectionalLight(DirectionalLightData& data, float nearZ, float farZ);
        glm::mat4 getLightSpaceMatrix() override;
        void setColor(glm::vec3& color) override;
        void setStrength(float strength) override;
        void setPosition(glm::vec3& position) override;
        void setDirection(glm::vec3& direction) override;
        void setProjectionSize(float projectionSize);
    private:
        glm::mat4 lightSpaceMatrix{};
        float projectionSize{ 10.0f };
        glm::vec3 position{};
    };
}
