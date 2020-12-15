#pragma once

#include "ShadowLog.h"
#include "TextureVertex.h"

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>

namespace shadow
{
    constexpr double PI{ 3.14159265358979323846 };
    constexpr float FPI{ 3.141592653f };
    class ShadowUtils final
    {
    public:
        ShadowUtils() = delete;
        static std::vector<glm::vec3> generateNormals(const std::vector<glm::vec3>& vertices, const std::vector<GLuint>& indices);
        static glm::vec3 getNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
        static void generateTangentsBitangents(std::vector<TextureVertex>& vert, const std::vector<GLuint>& indices);
        template<typename T> static T min3(T a, T b, T c);
        template<typename T> static T max3(T a, T b, T c);
    };

    template<typename T> T ShadowUtils::min3(T a, T b, T c)
    {
        return (a < b) ? (a < c ? a : c) : (b < c ? b : c);
    }

    template<typename T> T ShadowUtils::max3(T a, T b, T c)
    {
        return (a > b) ? (a > c ? a : c) : (b > c ? b : c);
    }
}
