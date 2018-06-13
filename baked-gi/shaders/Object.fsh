#include "BRDF.glsl"

in vec3 vWorldPos;
in vec3 vNormal;
in vec4 vTangent;
in vec2 vTexCoord;
in vec2 vLightMapTexCoord;

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
uniform sampler2D uTextureRoughness;
uniform sampler2D uTextureNormal;
uniform sampler2D uTextureIrradiance;

vec3 unpackNormalMap(vec3 rgb) {
	return vec3(rgb.rg * 2 - 1, rgb.b);
}

void main() {
	vec3 N = normalize(vNormal);
	vec3 T = normalize(vTangent.xyz) * vTangent.w;
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	// Texturing
	vec3 color = texture(uTextureColor, vTexCoord).rgb * uBaseColor;
	float roughness = texture(uTextureRoughness, vTexCoord).r * uRoughness;
    vec3 normal = unpackNormalMap(texture(uTextureNormal, vTexCoord).rgb);
    N = normalize(TBN * normal);
	vec3 irradiance = texture(uTextureIrradiance, vLightMapTexCoord).rgb;

	// Shading
	vec3 V = normalize(uCamPos - vWorldPos);
	vec3 L = uLightDir;
    fColor = uAmbientColor * color + irradiance * color + shadingGGX(N, V, L, color, roughness, uMetallic) * uLightColor;

	// Extract brightness for bloom
	float brightness = dot(fColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        fBrightColor = fColor;
    }
	else {
        fBrightColor = vec3(0.0);
	}
}