#pragma once

#include "ShadowLog.h"

#include "glad/glad.h"
#include <glm/glm.hpp>

namespace shadow
{
    class Framebuffer
    {
    public:
        Framebuffer() = default;
        ~Framebuffer();
        Framebuffer(Framebuffer&) = delete;
        Framebuffer(Framebuffer&&) = delete;
        Framebuffer& operator=(Framebuffer&) = delete;
        Framebuffer& operator=(Framebuffer&&) = delete;
        bool initialize(bool addDepthRenderbuffer, GLenum attachment, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLint filter, GLint wrappingTechnique, glm::vec4 border = glm::vec4(0.0f));
        void resize(GLsizei width, GLsizei height);
        inline GLuint getTexture() const;
        inline GLuint getFbo() const;
    private:
        static GLuint createTexture(GLenum attachment, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLint filter, GLint wrappingTechnique, glm::vec4 border);
        static GLuint createDepthRenderbuffer(GLsizei width, GLsizei height);
        GLuint framebuffer{}, texture{}, depthRenderbuffer{};
        GLint internalFormat{}, filter{}, wrappingTechnique{};
        GLsizei width{}, height{};
        GLenum attachment{}, format{}, type{};
        glm::vec4 border{};
    };

    inline GLuint shadow::Framebuffer::getTexture() const
    {
        assert(texture);
        return texture;
    }

    inline GLuint shadow::Framebuffer::getFbo() const
    {
        assert(framebuffer);
        return framebuffer;
    }
}
