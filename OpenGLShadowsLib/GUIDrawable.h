#pragma once

#include "imgui.h"

namespace shadow
{
    class GUIDrawable abstract
    {
    public:
        GUIDrawable() = default;
        virtual ~GUIDrawable() = default;
        virtual void drawGui() = 0;
    };
}