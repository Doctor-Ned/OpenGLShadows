#pragma once

#include "ShadowLog.h"
#include "glad/glad.h"

#define GL_PUSH_DEBUG_GROUP(name)                                      \
do {                                                                   \
    const char* DEBUG_GROUP_NAME = #name;                              \
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0,                   \
    static_cast<GLsizei>(strlen(DEBUG_GROUP_NAME)), DEBUG_GROUP_NAME); \
} while(false)

#define GL_POP_DEBUG_GROUP() glPopDebugGroup()