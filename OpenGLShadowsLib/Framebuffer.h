#pragma once

#include "ShadowLog.h"

#include "glad/glad.h"

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
        bool create(bool createDepthRenderbuffer, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type);
        void resize(GLsizei width, GLsizei height);
        inline GLuint getTexture() const;
        inline GLuint getFbo() const;
    private:
        static GLuint createTexture(GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type);
        GLuint framebuffer{}, texture{}, depthRenderbuffer{};
        GLint internalFormat{};
        GLsizei width{}, height{};
        GLenum format{}, type{};
    };
}
