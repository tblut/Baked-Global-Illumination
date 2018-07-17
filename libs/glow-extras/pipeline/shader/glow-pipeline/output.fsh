#include "utils.glsl"

uniform sampler2DRect uTexture;

uniform vec2 uOutputSize;
uniform float uDitheringStrength;

in vec2 aPosition;

out vec3 fColor;

void main()
{
    // this is tested to properly down-scale 2-to-1
    vec3 color = texture(uTexture, gl_FragCoord.xy * textureSize(uTexture) / uOutputSize).rgb;

    uint seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * 8096;
    float r = wang_float(wang_hash(seed * 3 + 0));
    float g = wang_float(wang_hash(seed * 3 + 1));
    float b = wang_float(wang_hash(seed * 3 + 2));
    vec3 random = vec3(r, g, b);

    fColor = color + (random - .5) * uDitheringStrength;
}
