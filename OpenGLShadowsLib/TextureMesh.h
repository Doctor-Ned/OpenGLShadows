#pragma once

#include "TextureVertex.h"
#include "TextureType.h"
#include "Mesh.h"
#include "Texture.h"

#include <vector>
#include <map>

namespace shadow
{
    class TextureMesh final : public Mesh
    {
    public:
        TextureMesh(const std::vector<TextureVertex>& vertices, const std::vector<GLuint>& indices,
                    std::map<TextureType, std::shared_ptr<Texture>> textures);
        void draw(std::shared_ptr<GLShader> shader) const override;
    private:
        std::map<TextureType, std::shared_ptr<Texture>> textures{};
        GLuint vao{}, vbo{}, ebo{};
        GLsizei indexCount{};
    };
}

