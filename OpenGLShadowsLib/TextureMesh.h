#pragma once

#include "TextureVertex.h"
#include "TextureType.h"
#include "Mesh.h"
#include "Texture.h"
#include "PrimitiveData.h"

#include <vector>
#include <map>

namespace shadow
{
    class TextureMesh final : public Mesh
    {
    public:
        TextureMesh(const std::vector<TextureVertex>& vertices, const std::vector<GLuint>& indices,
                    std::map<TextureType, std::shared_ptr<Texture>> textures);
        TextureMesh(const std::vector<TextureVertex>& vertices, const std::vector<GLuint>& indices,
                    std::shared_ptr<Texture> texture);
        ~TextureMesh();
        static std::shared_ptr<TextureMesh> fromPrimitiveData(std::shared_ptr<PrimitiveData> data,
                                                              std::map<TextureType, std::shared_ptr<Texture>> textures);
        void draw(std::shared_ptr<GLShader> shader) const override;
        ShaderType getShaderType() const override;
    private:
        std::map<TextureType, std::shared_ptr<Texture>> textures{};
        GLuint vao{}, vbo{}, ebo{};
        GLsizei indexCount{};
    };
}

