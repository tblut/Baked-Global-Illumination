#include <glow-pipeline/transparency.glsl>

uniform vec4 uColor;

void main()
{
    outputTransparency(uColor.rgb * uColor.a, uColor.a);
}