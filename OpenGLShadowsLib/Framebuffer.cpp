#include "Framebuffer.h"

bool shadow::Framebuffer::create(bool createDepthRenderbuffer, GLenum attachment, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type)
    : internalFormat(internalFormat), width(width), height(height), attachment(attachment), format(format), type(type)
{
    SHADOW_DEBUG("Creating {}x{} framebuffer ({}, {}, {}, {}, {})...", width, height, createDepthRenderbuffer, attachment, internalFormat, format, type);
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    texture = createTexture(attachment, internalFormat, width, height, format, type);
    if (createDepthRenderbuffer)
    {
        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    }
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
    texture = createTexture(attachment, internalFormat, width, height, format, type);
    this->width = width;
    this->height = height;
    glDeleteTextures(1, &oldTexture);
}

GLuint shadow::Framebuffer::createTexture(GLenum attachment, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
    if (attachment == GL_DEPTH_ATTACHMENT)
    {
        glDrawBuffer(GL_NONE);
    } else
    {
        glDrawBuffer(attachment);
    }
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
