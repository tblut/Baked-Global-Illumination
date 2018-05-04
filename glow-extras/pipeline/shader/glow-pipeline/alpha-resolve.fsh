uniform sampler2DRect uTexAccum;
uniform sampler2DRect uTexRevealage;

out vec4 fColor;

void main()
{
    vec4 accum = texelFetch(uTexAccum, ivec2(gl_FragCoord.xy)).rgba;
    float reveal = texelFetch(uTexRevealage, ivec2(gl_FragCoord.xy)).r;

    fColor.rgb = accum.rgb / clamp(accum.a, 1e-4, 5e4);
    fColor.a = reveal;
}
