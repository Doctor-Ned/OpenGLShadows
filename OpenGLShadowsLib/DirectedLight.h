#pragma once

#include "Light.h"

namespace shadow
{
    template<typename T>
    class DirectedLight abstract : public Light<T>
    {
    public:
        virtual void setDirection(glm::vec3& direction) = 0;
        virtual glm::vec3 getDirection() const = 0;
    };
}