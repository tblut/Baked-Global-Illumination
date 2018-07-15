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
    //vec3 R = reflect(-V, N);//2 * dot(N, V) * N - V;

    vec3 specularColor = mix(vec3(0.04), color, metallic);

    float maxLevel = floor(log2(float(textureSize(uEnvMapGGX, 0).x)));
    vec2 envbrdf = textureLod(uEnvLutGGX, vec2(roughness, dotNV), 0).xy;
    vec3 envcolor = textureLod(uEnvMapGGX, R, roughness * maxLevel).rgb;

    return envcolor * (specularColor * envbrdf.x + envbrdf.y);
}

vec3 iblSpecularGGXProbe(vec3 N, vec3 V, vec3 R, vec3 color, float roughness, float metallic, vec3 worldPos) {
    float dotNV = max(dot(N, V), 0.0);
    //vec3 R = reflect(-V, N);//2 * dot(N, V) * N - V;

    vec3 specularColor = mix(vec3(0.04), color, metallic);

    float maxLevel = floor(log2(float(textureSize(uEnvMapGGX, 0).x)));
    vec2 envbrdf = textureLod(uEnvLutGGX, vec2(roughness, dotNV), 0).xy;
	
	vec3 gridCell = getProbeGridCell(worldPos);
	vec3 probeLayers = getProbeLayersForVoxel(gridCell);

	vec3 probePos0 = getProbePositionForLayer(probeLayers.x);
	vec3 probePos1 = getProbePositionForLayer(probeLayers.y);
	vec3 probePos2 = getProbePositionForLayer(probeLayers.z);

	vec3 boxSize = vec3(20);
	vec3 aabbMin0 = probePos0 - boxSize;//getProbeAABBMin(probeLayers.x);
	vec3 aabbMax0 = probePos0 + boxSize;//getProbeAABBMax(probeLayers.x);
	vec3 aabbMin1 = probePos1 - boxSize;//getProbeAABBMin(probeLayers.y);
	vec3 aabbMax1 = probePos1 + boxSize;//getProbeAABBMax(probeLayers.y);
	vec3 aabbMin2 = probePos2 - boxSize;//getProbeAABBMin(probeLayers.z);
	vec3 aabbMax2 = probePos2 + boxSize;//getProbeAABBMax(probeLayers.z);

	vec3 R0 = parallaxCorrectedReflection(R, worldPos, probePos0, aabbMin0, aabbMax0);
	vec3 R1 = parallaxCorrectedReflection(R, worldPos, probePos1, aabbMin1, aabbMax1);
	vec3 R2 = parallaxCorrectedReflection(R, worldPos, probePos2, aabbMin2, aabbMax2);

	vec3 envcolor0 = textureLod(uReflectionProbeArray, vec4(R, probeLayers.x), roughness * maxLevel).rgb;
	vec3 envcolor1 = textureLod(uReflectionProbeArray, vec4(R, probeLayers.y), roughness * maxLevel).rgb;
	vec3 envcolor2 = textureLod(uReflectionProbeArray, vec4(R, probeLayers.z), roughness * maxLevel).rgb;

	vec3 envcolor = vec3(1,0,1);// = (envcolor0 + envcolor1 + envcolor2) / 3.0;
	if (isInInnerBox(worldPos, aabbMin0, aabbMax0)) envcolor = envcolor0;
	else if (isInInnerBox(worldPos, aabbMin1, aabbMax1)) envcolor = envcolor1;
	else if (isInInnerBox(worldPos, aabbMin2, aabbMax2)) envcolor = envcolor2;
	else {
		bool isInBox0 = isInBox(worldPos, aabbMin0, aabbMax0);
		bool isInBox1 = isInBox(worldPos, aabbMin1, aabbMax1);
		bool isInBox2 = isInBox(worldPos, aabbMin2, aabbMax2);

		if (isInBox0 && isInBox1 && isInBox2) {
			vec3 blendFactors = getBlendMapFactors3(worldPos, aabbMin0, aabbMax0, aabbMin1, aabbMax1, aabbMin2, aabbMax2);
			envcolor = envcolor0 * blendFactors.x + envcolor1 * blendFactors.y + envcolor2 * blendFactors.z;
		}
		else if (isInBox0 && isInBox1) {
			vec2 blendFactors = getBlendMapFactors2(worldPos, aabbMin0, aabbMax0, aabbMin1, aabbMax1);
			envcolor = envcolor0 * blendFactors.x + envcolor1 * blendFactors.y;
		}
		else if (isInBox0 && isInBox2) {
			vec2 blendFactors = getBlendMapFactors2(worldPos, aabbMin0, aabbMax0, aabbMin2, aabbMax2);
			envcolor = envcolor0 * blendFactors.x + envcolor2 * blendFactors.y;
		}
		else if (isInBox1 && isInBox2) {
			vec2 blendFactors = getBlendMapFactors2(worldPos, aabbMin1, aabbMax1, aabbMin2, aabbMax2);
			envcolor = envcolor1 * blendFactors.x + envcolor2 * blendFactors.y;
		}
		else if (isInBox0) {
			float blendFactor = getBlendMapFactors1(worldPos, aabbMin0, aabbMax0);
			envcolor = envcolor0 * blendFactor;
		}
		else if (isInBox1) {
			float blendFactor = getBlendMapFactors1(worldPos, aabbMin1, aabbMax1);
			envcolor = envcolor1 * blendFactor;
		}
		else if (isInBox2) {
			float blendFactor = getBlendMapFactors1(worldPos, aabbMin2, aabbMax2);
			envcolor = envcolor2 * blendFactor;
		}
	}

	//envcolor = probeLayers;

    return envcolor * (specularColor * envbrdf.x + envbrdf.y);
}
