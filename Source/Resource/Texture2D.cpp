#include "pch.h"
#include "Texture2D.h"
#include "Graphics/Graphics.h"

namespace Dive
{
    Texture2D::Texture2D(uint32_t width, uint32_t height)
        : Texture(width, height)
    {
    }

    Texture2D::~Texture2D() = default;
}