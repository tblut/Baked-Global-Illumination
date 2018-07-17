// This file contains output declarations and helpers for the transparency stage
// Usage:
//   #include <glow-pipeline/transparency.glsl>
//   ...
//   outputTransparency(premultiplied-color, alpha);
//
// NOTE: the color has to be premultiplied by alpha. If you are using alpha-blended textures, have them premultiplied as well (otherwise Mipmapping is wrong)
//       If you want to model emission, you can add that (without multiplying by alpha) to the color

// additive accum buffer
out vec4 fAccum;
// multiplicative revealage buffer
out float fRevealage;

void outputTransparency(vec3 premulColor, float alpha)
{
    // using weight eq. (10)
    float d = 1 - gl_FragCoord.z;
    float w = alpha * max(1e-2, 3e3 * d * d * d);
    fAccum = vec4(premulColor, alpha) * w;
    fRevealage = alpha;
}

void outputFresnel(vec3 N, vec3 V, vec3 premulColor, float baseAlpha)
{
    float dotNV = dot(N, V);

    float F = pow(1.0 - abs(dotNV), 5.0); // manually?

    float a = mix(baseAlpha, 1.0, F);

    outputTransparency(premulColor / baseAlpha * a, a);
}

