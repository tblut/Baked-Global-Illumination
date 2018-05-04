// see http://www.gamedev.net/topic/655431-ibl-problem-with-consistency-using-ggx-anisotropy/

#include "precalc.glsl"

// http://www.unrealengine.com/files/downloads/2013SiggraphPresentationsNotes.pdf
vec2 IntegrateBRDF( float Roughness, float dotNV )
{
    vec3 V = vec3( sqrt(1 - dotNV * dotNV), // sin
                   0.0,
                   dotNV ); // cos

    float A = 0;
    float B = 0;

    vec3 N = vec3(0, 0, 1);

    float k = Roughness / 2.0;
    k = k * k;

    const int samples = 1024;
    for (int i = 0; i < samples; ++i)
    {
        vec2 Xi = Hammersley(i, samples);
        vec3 H = ImportanceSampleGGX( Xi, Roughness, N );
        vec3 L = 2 * dot(V, H) * H - V;

        float dotNL = max(L.z, 0.0);
        float dotNH = max(H.z, 0.0);
        float dotVH = max(dot(V, H), 0.0);

        if (dotNL > 0)
        {
            // original:
            // float G = dotNL * dotNV / (mix(dotNV, 1, k) * mix(dotNL, 1, k));
            // float G_Vis = G * dotVH / (dotNH * dotNV);

            // slightly optimized
            float G = dotNL / (mix(dotNV, 1, k) * mix(dotNL, 1, k));
            float G_Vis = G * dotVH / dotNH;

            float Fc = pow(1 - dotVH, 5);

            A += (1 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    return vec2(A, B) / float(samples);
}


/// COMPUTE SHADER
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

uniform layout(rg16f, binding=0) writeonly image2DRect uLUT;

void main()
{
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;
    ivec2 s = imageSize(uLUT);

    if (x >= s.x || y >= s.y)
        return; // out of bounds

    vec2 lut = IntegrateBRDF((float(x) + .5) / float(s.x), (float(y) + .5) / float(s.y));

    imageStore(uLUT, ivec2(x, y), vec4(lut, 0, 0));
}
