#include "BRDF.glsl"

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vTangent;
in vec2 vTexCoord;

out vec3 fColor;
out vec3 fBrightColor;

uniform vec3 uCamPos;
uniform vec3 uBaseColor;
uniform float uRoughness;
uniform float uMetallic;
uniform vec3 uAmbientColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;

uniform sampler2D uTextureColor;
uniform sampler2D uTextureNormal;

vec3 unpackNormalmap(vec3 rgb) {
  return vec3(rgb.rg * 2 - 1 - 1 / 256, rgb.b);
}

void main() {
	vec3 N = normalize(vNormal);
	vec3 T = normalize(vTangent);
	vec3 B = normalize(cross(T, N));
	mat3 TBN = mat3(T, B, N);

	// Texturing
	vec3 color = texture(uTextureColor, vTexCoord).rgb * uBaseColor;
    vec3 normal = unpackNormalmap(texture(uTextureNormal, vTexCoord).rgb);
    N = normalize(TBN * normal);

	// Shading
	vec3 V = normalize(uCamPos - vWorldPos);
	vec3 L = normalize(-uLightDir);
    fColor = uAmbientColor * color + shadingGGX(N, V, L, color, uRoughness, uMetallic) * uLightColor;

	// Extract brightness for bloom
	float brightness = dot(fColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        fBrightColor = fColor;
    }
	else {
        fBrightColor = vec3(0.0);
	}
}