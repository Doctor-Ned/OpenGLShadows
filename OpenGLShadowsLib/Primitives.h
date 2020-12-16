#pragma once

#include "PrimitiveData.h"

#include <memory>

namespace shadow
{
    class Primitives final
    {
    public:
        Primitives() = delete;
        static std::shared_ptr<PrimitiveData> cuboid();
        static std::shared_ptr<PrimitiveData> cuboid(float width, float height, float length);
        static std::shared_ptr<PrimitiveData> cube();
        static std::shared_ptr<PrimitiveData> cube(float size);
        static std::shared_ptr<PrimitiveData> cone(unsigned int precision);
        static std::shared_ptr<PrimitiveData> cone(unsigned int precision, float radius, float height);
        static std::shared_ptr<PrimitiveData> cylinder(unsigned int precision, unsigned int heightSegments);
        static std::shared_ptr<PrimitiveData> cylinder(unsigned int precision, unsigned int heightSegments, float radius, float height);
        static std::shared_ptr<PrimitiveData> plane();
        static std::shared_ptr<PrimitiveData> plane(float width, float length, glm::vec2 texCoordsMax);
        static std::shared_ptr<PrimitiveData> sphere(unsigned int precision);
        static std::shared_ptr<PrimitiveData> sphere(unsigned int precision, float radius);
        static std::shared_ptr<PrimitiveData> torus(unsigned int precision);
        static std::shared_ptr<PrimitiveData> torus(unsigned int precision, float innerRadius, float outerRadius);
    private:
        static std::vector<glm::vec3> createHorizontalCircle(unsigned int precision, float radius, float x, float y, float z);
        static std::vector<glm::vec3> createVerticalCircle(unsigned int precision, float radius, float horizontalAngle, float x, float y, float z);
    };
}
