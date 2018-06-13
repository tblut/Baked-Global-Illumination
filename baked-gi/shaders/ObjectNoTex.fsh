#include "BRDF.glsl"

in vec3 vWorldPos;
in vec3 vNormal;
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

uniform sampler2D uTextureIrradiance;

void main() {
	vec3 N = normalize(vNormal);
	vec3 V = normalize(uCamPos - vWorldPos);
	vec3 L = uLightDir;

	// Shading
	vec3 irradiance = texture(uTextureIrradiance, vLightMapTexCoord).rgb;
	fColor = irradiance * (1.0 - uMetallic) * uBaseColor;// + shadingGGX(N, V, L, uBaseColor, uRoughness, uMetallic) * uLightColor;

	// Extract brightness for bloom
	float brightness = dot(fColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        fBrightColor = fColor;
    }
	else {
        fBrightColor = vec3(0.0);
	}
}
