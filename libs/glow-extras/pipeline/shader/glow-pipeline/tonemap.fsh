#include "utils.glsl"

uniform sampler2DRect uTexture;

out vec3 fColor;

void main()
{
    vec3 color = texelFetch(uTexture, ivec2(gl_FragCoord.xy)).rgb;

    // TODO: tonemap
    color = clamp(color, vec3(0), vec3(1));

    fColor = linearToSRGB(color); // linear to sRGB conversion
}
