#include "Framebuffer.h"
#include <glm/gtc/type_ptr.hpp>

bool shadow::Framebuffer::initialize(bool addDepthRenderbuffer, GLenum attachment, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLint filter, GLint wrappingTechnique, glm::vec4 border)
{
    if (width <= 0 || height <= 0)
    {
        SHADOW_ERROR("Invalid framebuffer size ({}x{})!", width, height);
        return false;
    }
    SHADOW_DEBUG("Creating {}x{} framebuffer ({}, {}, {}, {}, {}, {}, {})...", width, height, addDepthRenderbuffer, attachment, internalFormat, format, type, filter, wrappingTechnique);
    this->attachment = attachment;
    this->internalFormat = internalFormat;
    this->width = width;
    this->height = height;
    this->format = format;
    this->type = type;
    this->filter = filter;
    this->wrappingTechnique = wrappingTechnique;
    this->border = border;
    GLint previousFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    texture = createTexture(attachment, internalFormat, width, height, format, type, filter, wrappingTechnique, border);
    if (addDepthRenderbuffer)
    {
        depthRenderbuffer = createDepthRenderbuffer(width, height);
    }
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        SHADOW_ERROR("Framebuffer initialization failed!");
        glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);
    return true;
}

void shadow::Framebuffer::resize(GLsizei width, GLsizei height)
{
    assert(framebuffer);
    assert(width > 0 && height > 0);
    if (this->width == width && this->height == height)
    {
        return;
    }
    SHADOW_DEBUG("Resizing framebuffer to {}x{}...", width, height);
    GLint previousFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);
    bool switchFramebuffer = previousFramebuffer != framebuffer;
    if (switchFramebuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    }
    GLuint oldTexture = texture;
    texture = createTexture(attachment, internalFormat, width, height, format, type, filter, wrappingTechnique, border);
    glDeleteTextures(1, &oldTexture);
    if (depthRenderbuffer)
    {
        GLuint oldDepthRenderbuffer = depthRenderbuffer;
        depthRenderbuffer = createDepthRenderbuffer(width, height);
        glDeleteRenderbuffers(1, &oldDepthRenderbuffer);
    }
    this->width = width;
    this->height = height;
    if (switchFramebuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);
    }
}

GLuint shadow::Framebuffer::createTexture(GLenum attachment, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLint filter, GLint wrappingTechnique, glm::vec4 border)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingTechnique);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingTechnique);
    if (wrappingTechnique == GL_CLAMP_TO_BORDER)
    {
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, value_ptr(border));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
    if (attachment == GL_DEPTH_ATTACHMENT)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    } else
    {
        glDrawBuffer(attachment);
    }
    return texture;
}

GLuint shadow::Framebuffer::createDepthRenderbuffer(GLsizei width, GLsizei height)
{
    GLuint depthRenderbuffer;
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    return depthRenderbuffer;
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
