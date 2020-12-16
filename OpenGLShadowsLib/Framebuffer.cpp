#include "Framebuffer.h"

bool shadow::Framebuffer::create(bool createDepthRenderbuffer, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type)
{
    SHADOW_DEBUG("Creating {}x{} framebuffer ({}, {}, {}, {})...", width, height, createDepthRenderbuffer, internalFormat, format, type);
    this->internalFormat = internalFormat;
    this->width = width;
    this->height = height;
    this->format = format;
    this->type = type;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    if (createDepthRenderbuffer)
    {
        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    }
    texture = createTexture(internalFormat, width, height, format, type);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        SHADOW_ERROR("Framebuffer initialization failed!");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void shadow::Framebuffer::resize(GLsizei width, GLsizei height)
{
    assert(framebuffer);
    SHADOW_DEBUG("Resizing framebuffer to {}x{}...", width, height);
    GLuint oldTexture = texture;
    texture = createTexture(internalFormat, width, height, format, type);
    this->width = width;
    this->height = height;
    glDeleteTextures(1, &oldTexture);
}

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

GLuint shadow::Framebuffer::createTexture(GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    return texture;
}

shadow::Framebuffer::~Framebuffer()
{
    if (depthRenderbuffer)
    {
        glDeleteRenderbuffers(1, &depthRenderbuffer);
    }
    glDeleteTextures(1, &texture);
    glDeleteFramebuffers(1, &framebuffer);
}
