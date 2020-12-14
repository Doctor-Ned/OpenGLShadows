#pragma once

#include "Light.h"

namespace shadow
{
    template<typename T>
    class DirectedLight abstract : public Light<T>
    {
    public:
        virtual void setDirection(glm::vec3& direction) = 0;
    protected:
        DirectedLight(T& data, float nearZ, float farZ) : Light<T>(data, nearZ, farZ) {}
    };
}