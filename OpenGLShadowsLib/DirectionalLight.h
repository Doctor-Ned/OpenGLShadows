#pragma once

#include "DirectedLight.h"

namespace shadow
{
    struct DirectionalLightData
    {
        glm::mat4 lightSpace{};
        glm::vec3 color{ 0.0f, 0.0f, 0.0f };
        float strength{ 0.0f };
        glm::vec3 direction{ 0.0f, 0.0f, -1.0f };
        float nearZ{ 0.1f };
        glm::vec2 padding;
        float farZ{ 10.0f };
        float lightSize{ 1.0f };
    };

    class DirectionalLight final : public DirectedLight<DirectionalLightData>
    {
    public:
        DirectionalLight(DirectionalLightData& data);
        glm::mat4 getLightSpace() override;
        void updateLightSpace() override;
        void setColor(glm::vec3 color) override;
        void setStrength(float strength) override;
        void setPosition(glm::vec3 position) override;
        void setDirection(glm::vec3 direction) override;
        void setNearZ(float nearZ) override;
        void setFarZ(float farZ) override;
        void setLightSize(float lightSize) override;
        void setProjectionSize(float projectionSize);
        float getProjectionSize() const;
    private:
        float projectionSize{ 10.0f };
        glm::vec3 position{};
        float angleX{}, angleY{}, angleZ{};
    };
}
