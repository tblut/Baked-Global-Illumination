#include "FXAA.glsl"

uint wang_hash(uint seed) {
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9u;
    seed = seed ^ (seed >> 4u);
    seed *= 0x27D4EB2Du;
    seed = seed ^ (seed >> 15u);
    return seed;
}

float wang_float(uint hash) {
    return hash / float(0x7FFFFFFF) / 2.0;
}

uniform sampler2DRect uHdrBuffer;
uniform sampler2DRect uBloomBuffer;

out vec3 fColor;

void main() {
	ivec2 coords = ivec2(gl_FragCoord.xy);

    // FXAA
    vec3 hdrColor = fxaa(uHdrBuffer, gl_FragCoord.xy).rgb;

    // Reinhard tone mapping
    vec3 ldrColor = hdrColor / (hdrColor + vec3(1.0));

	// Apply bloom
	vec3 bloomColor = texture(uBloomBuffer, gl_FragCoord.xy / 2).rgb;
	ldrColor += bloomColor * 0.5;

    // Gamma correction 
    ldrColor = pow(ldrColor, vec3(1.0 / 2.2));

	// Dithering
    uint seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * 8096u;
    float r = wang_float(wang_hash(seed * 3u + 0u));
    float g = wang_float(wang_hash(seed * 3u + 1u));
    float b = wang_float(wang_hash(seed * 3u + 2u));
    vec3 random = vec3(r, g, b);
    ldrColor += (random - 0.5) / 256.0;

    fColor = ldrColor;
}