#include "BRDF.glsl"
#include "Shadow.glsl"

in vec3 vWorldPos;
in vec3 vNormal;
#ifdef TEXTURE_MAPPING
in vec4 vTangent;
in vec2 vTexCoord;
#endif
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
uniform bool uUseIBL;
uniform float uDirectLightingFade;
uniform float uIrradianceFade;
uniform float uIBLFade;
uniform float uLocalProbesFade;

uniform vec3 uProbePos;
uniform vec3 uAABBMin;
uniform vec3 uAABBMax;

uniform sampler2D uTextureIrradiance;
uniform sampler2D uTextureAO;

#ifdef TEXTURE_MAPPING
uniform sampler2D uTextureColor;
uniform sampler2D uTextureRoughness;
uniform sampler2D uTextureNormal;

vec3 unpackNormalMap(vec3 rgb) {
	return vec3(rgb.rg * 2 - 1, rgb.b);
}
#endif

void main() {
	// Texture mapping
#ifdef TEXTURE_MAPPING
    vec3 N = normalize(vNormal);
	vec3 T = normalize(vTangent.xyz) * vTangent.w;
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);
	vec3 normal = unpackNormalMap(texture(uTextureNormal, vTexCoord).rgb);
    N = normalize(TBN * normal);
    
	vec3 color = texture(uTextureColor, vTexCoord).rgb * uBaseColor;
	float roughness = texture(uTextureRoughness, vTexCoord).r * uRoughness;
#else
    vec3 N = normalize(vNormal);
    vec3 color = uBaseColor;
    float roughness = uRoughness;
#endif
	
	// Shadow
	float shadowFactor = calcShadowFactor(vLightSpacePos);

	// Shading
    vec3 V = normalize(uCamPos - vWorldPos);
	vec3 L = uLightDir;
	vec3 direct = shadingGGX(N, V, L, color, roughness, uMetallic) * uLightColor * shadowFactor * uDirectLightingFade;
    
#ifdef IBL
	if (uUseIBL) {
		vec3 R = reflect(-V, N);
	#ifdef LOCAL_PROBES
		direct += iblSpecularGGXProbe(N, V, R, color, roughness, uMetallic, vWorldPos) * uIBLFade * uLocalProbesFade;
	#endif
	//#else
		direct += iblSpecularGGX(N, V, R, color, roughness, uMetallic) * uIBLFade * (1.0 - uLocalProbesFade);
	//#endif
	}
#endif

	vec3 indirect = vec3(0.0);
	if (uUseIrradianceMap) {
		vec3 irradiance = texture(uTextureIrradiance, vLightMapTexCoord).rgb;
		vec3 diffuse = (1.0 - uMetallic) * color;
		indirect += irradiance * diffuse * uIrradianceFade;
	}
	if (uUseAOMap) {
		float ao = texture(uTextureAO, vLightMapTexCoord).r;
		direct *= ao;
	}
	vec3 finalColor = direct + indirect;

	fColor = finalColor * (1.0 - uBloomPercentage);
	fBrightColor = finalColor * uBloomPercentage;
}
