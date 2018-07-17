#pragma once

namespace glow
{
/**
 * @brief Color space of a given texture
 */
enum class ColorSpace
{
    Linear,
    sRGB

    // TODO: in the future, have a proper autodetect?
};
}
