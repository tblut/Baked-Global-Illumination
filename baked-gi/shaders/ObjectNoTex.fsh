#include "BRDF.glsl"

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
uniform vec2 uShadowMapSize;
uniform float uShadowOffset;

uniform sampler2D uTextureIrradiance;
uniform sampler2D uTextureAO;
uniform sampler2DRectShadow uTextureShadow;

void main() {
	vec3 N = normalize(vNormal);
	vec3 V = normalize(uCamPos - vWorldPos);
	vec3 L = uLightDir;

	// Shadow
	vec3 projCoords = (vLightSpacePos.xyz / vLightSpacePos.w) * 0.5 + 0.5;
	projCoords.xy *= uShadowMapSize;
	projCoords.z -= uShadowOffset;

	float shadowFactor = 0.0;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(x, y), projCoords.z));      
		}    
	}
	shadowFactor /= 9.0;


	// Shading
	//vec3 ao = texture(uTextureAO, vLightMapTexCoord).rgb;
	vec3 irradiance = texture(uTextureIrradiance, vLightMapTexCoord).rgb;
	vec3 diffuse = (1.0 - uMetallic) * uBaseColor;
	fColor = irradiance * diffuse /** ao*/ + shadingGGX(N, V, L, uBaseColor, uRoughness, uMetallic) * uLightColor * shadowFactor;

	// Extract brightness for bloom
	float brightness = dot(fColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        fBrightColor = fColor;
    }
	else {
        fBrightColor = vec3(0.0);
	}
}
