#include "ProbeGrid.glsl"
#include "CubeMapUtils.glsl"

uniform samplerCube uEnvMapGGX;
uniform sampler2D uEnvLutGGX;
uniform samplerCubeArray uReflectionProbeArray;

// DO NOT MULTIPLY BY COS THETA
vec3 shadingSpecularGGX(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0) {
    // see http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/
    vec3 H = normalize(V + L);

    float dotLH = max(dot(L, H), 0.0);
    float dotNH = max(dot(N, H), 0.0);
    float dotNL = max(dot(N, L), 0.0);
    float dotNV = max(dot(N, V), 0.0);

    float alpha = roughness * roughness;

    // D (GGX normal distribution)
    float alphaSqr = alpha * alpha;
    float denom = dotNH * dotNH * (alphaSqr - 1.0) + 1.0;
    float D = alphaSqr / (denom * denom);
    // no pi because BRDF -> lighting

    // F (Fresnel term)
    float F_a = 1.0;
    float F_b = pow(1.0 - dotLH, 5);
    vec3 F = mix(vec3(F_b), vec3(F_a), F0);

    // G (remapped hotness, see Unreal Shading)
    float k = (alpha + 2 * roughness + 1) / 8.0;
    float G = dotNL / (mix(dotNL, 1, k) * mix(dotNV, 1, k));
    // '* dotNV' - canceled by normalization

    // '/ dotLN' - canceled by lambert
    // '/ dotNV' - canceled by G
    return D * F * G / 4.0;
}

vec3 shadingGGX(vec3 N, vec3 V, vec3 L, vec3 color, float roughness, float metallic) {
	vec3 diffuse = color * (1 - metallic);
    vec3 specular = mix(vec3(0.04), color, metallic);
	vec3 shadingDiffuse = diffuse * max(dot(N, L), 0.0);
    return shadingDiffuse + shadingSpecularGGX(N, V, L, max(0.01, roughness), specular);
}

vec3 iblSpecularGGX(vec3 N, vec3 V, vec3 R, vec3 color, float roughness, float metallic) {
    float dotNV = max(dot(N, V), 0.0);
    vec3 specularColor = mix(vec3(0.04), color, metallic);

    float maxLevel = floor(log2(float(textureSize(uEnvMapGGX, 0).x)));
    vec2 envbrdf = textureLod(uEnvLutGGX, vec2(roughness, dotNV), 0).xy;
    vec3 envcolor = textureLod(uEnvMapGGX, R, roughness * maxLevel).rgb;

    return envcolor * (specularColor * envbrdf.x + envbrdf.y);
}

vec3 iblSpecularGGXProbe(vec3 N, vec3 V, vec3 R, vec3 color, float roughness, float metallic, vec3 worldPos) {
    float dotNV = max(dot(N, V), 0.0);
    vec3 specularColor = mix(vec3(0.04), color, metallic);

    float maxLevel = floor(log2(float(textureSize(uReflectionProbeArray, 0).x)));
    vec2 envbrdf = textureLod(uEnvLutGGX, vec2(roughness, dotNV), 0).xy;
	
	vec3 gridCell = getProbeGridCell(worldPos);
	vec3 probeLayers = getProbeLayersForVoxel(gridCell);

	if (probeLayers.x < 0 && probeLayers.y < 0 && probeLayers.z < 0) {
		return iblSpecularGGX(N, V, R, color, roughness, metallic);
	}
	else {
		vec3 probePos0 = getProbePosition(probeLayers.x);
		vec3 probePos1 = getProbePosition(probeLayers.y);
		vec3 probePos2 = getProbePosition(probeLayers.z);

		vec3 probeBoxMin0 = probePos0 + getProbeInfluenceBoxMin(probeLayers.x);
		vec3 probeBoxMax0 = probePos0 + getProbeInfluenceBoxMax(probeLayers.x);
		vec3 probeBoxMin1 = probePos1 + getProbeInfluenceBoxMin(probeLayers.y);
		vec3 probeBoxMax1 = probePos1 + getProbeInfluenceBoxMax(probeLayers.y);
		vec3 probeBoxMin2 = probePos2 + getProbeInfluenceBoxMin(probeLayers.z);
		vec3 probeBoxMax2 = probePos2 + getProbeInfluenceBoxMax(probeLayers.z);

		vec3 R0 = parallaxCorrectedReflection(R, worldPos, probePos0, probeBoxMin0, probeBoxMax0);
		vec3 R1 = parallaxCorrectedReflection(R, worldPos, probePos1, probeBoxMin1, probeBoxMax1);
		vec3 R2 = parallaxCorrectedReflection(R, worldPos, probePos2, probeBoxMin2, probeBoxMax2);

		vec3 envcolor0 = textureLod(uReflectionProbeArray, vec4(R0, probeLayers.x), roughness * maxLevel).rgb;
		vec3 envcolor1 = textureLod(uReflectionProbeArray, vec4(R1, probeLayers.y), roughness * maxLevel).rgb;
		vec3 envcolor2 = textureLod(uReflectionProbeArray, vec4(R2, probeLayers.z), roughness * maxLevel).rgb;

		vec3 blendFactors = getBlendMapFactors(worldPos, probeBoxMin0, probeBoxMax0, probeBoxMin1, probeBoxMax1, probeBoxMin2, probeBoxMax2);
		vec3 envcolor = envcolor0 * blendFactors.x + envcolor1 * blendFactors.y + envcolor2 * blendFactors.z;

		//envcolor = probeLayers;

		return envcolor * (specularColor * envbrdf.x + envbrdf.y);
	}
}
