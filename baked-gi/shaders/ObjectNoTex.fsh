#include "BRDF.glsl"
#include "Shadow.glsl"
#include "CubeMapUtils.glsl"

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

uniform float uBloomPercentage;
uniform bool uUseIrradianceMap;
uniform bool uUseAOMap;

uniform vec3 uProbePos;
uniform vec3 uAABBMin;
uniform vec3 uAABBMax;

uniform sampler2D uTextureIrradiance;
uniform sampler2D uTextureAO;

void main() {
	vec3 N = normalize(vNormal);
	vec3 V = normalize(uCamPos - vWorldPos);
	vec3 L = uLightDir;

	// Shadow
	float shadowFactor = calcShadowFactor(vLightSpacePos);

	// Shading
	vec3 direct = shadingGGX(N, V, L, uBaseColor, uRoughness, uMetallic) * uLightColor * shadowFactor;
    
    vec3 R = reflect(-V, N);
    R = parallaxCorrectedReflection(R, vWorldPos, uProbePos, uAABBMin, uAABBMax);
	direct += iblSpecularGGX(N, V, R, uBaseColor, uRoughness, uMetallic);

	vec3 indirect = vec3(0.0);
	if (uUseIrradianceMap) {
		vec3 irradiance = texture(uTextureIrradiance, vLightMapTexCoord).rgb;
		vec3 diffuse = (1.0 - uMetallic) * uBaseColor;
		indirect += irradiance * diffuse;
	}
	if (uUseAOMap) {
		float ao = texture(uTextureAO, vLightMapTexCoord).r;
		direct *= ao;
	}
	vec3 color = direct + indirect;

	fColor = color * (1.0 - uBloomPercentage);
	fBrightColor = color * uBloomPercentage;
}
