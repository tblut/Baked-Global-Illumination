#include "BRDF.glsl"
#include "Shadow.glsl"

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vLightMapTexCoord;
in vec4 vLightSpacePos;

out vec3 fColor;
out vec3 fBrightColor;

uniform vec3 uCamPos;
uniform vec3 uBaseColor;
uniform float uRoughness;
uniform float uMetallic;
uniform vec3 uAmbientColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;

uniform sampler2D uTextureIrradiance;
uniform sampler2D uTextureAO;

void main() {
	vec3 N = normalize(vNormal);
	vec3 V = normalize(uCamPos - vWorldPos);
	vec3 L = uLightDir;

	// Shadow
	float shadowFactor = calcShadowFactor(vLightSpacePos);

	// Shading
	float ao = texture(uTextureAO, vLightMapTexCoord).r;
	vec3 irradiance = texture(uTextureIrradiance, vLightMapTexCoord).rgb;
	vec3 diffuse = (1.0 - uMetallic) * uBaseColor;
	fColor = irradiance * diffuse * ao + shadingGGX(N, V, L, uBaseColor, uRoughness, uMetallic) * uLightColor * shadowFactor;

	// Extract brightness for bloom
	float brightness = dot(fColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        fBrightColor = fColor;
    }
	else {
        fBrightColor = vec3(0.0);
	}
}
