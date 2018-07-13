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

	vec3 aabbMin0 = probePos0 - vec3(5);//getProbeAABBMin(probeLayers.x);
	vec3 aabbMax0 = probePos0 + vec3(5);//getProbeAABBMax(probeLayers.x);
	vec3 aabbMin1 = probePos1 - vec3(5);//getProbeAABBMin(probeLayers.y);
	vec3 aabbMax1 = probePos1 + vec3(5);//getProbeAABBMax(probeLayers.y);
	vec3 aabbMin2 = probePos2 - vec3(5);//getProbeAABBMin(probeLayers.z);
	vec3 aabbMax2 = probePos2 + vec3(5);//getProbeAABBMax(probeLayers.z);

	vec3 R0 = parallaxCorrectedReflection(R, worldPos, probePos0, aabbMin0, aabbMax0);
	vec3 R1 = parallaxCorrectedReflection(R, worldPos, probePos1, aabbMin1, aabbMax1);
	vec3 R2 = parallaxCorrectedReflection(R, worldPos, probePos2, aabbMin2, aabbMax2);

	vec3 envcolor0 = textureLod(uReflectionProbeArray, vec4(R, probeLayers.x), roughness * maxLevel).rgb;
	vec3 envcolor1 = textureLod(uReflectionProbeArray, vec4(R, probeLayers.y), roughness * maxLevel).rgb;
	vec3 envcolor2 = textureLod(uReflectionProbeArray, vec4(R, probeLayers.z), roughness * maxLevel).rgb;

	vec3 envcolor = vec3(1,0,1);// = (envcolor0 + envcolor1 + envcolor2) / 3.0;
	if (isInInnerBox(worldPos, aabbMin0, aabbMax0)) envcolor = envcolor0;
	if (isInInnerBox(worldPos, aabbMin1, aabbMax1)) envcolor = envcolor1;
	if (isInInnerBox(worldPos, aabbMin2, aabbMax2)) envcolor = envcolor2;

	/*
	float layer000 = getProbeLayer(gridCell + vec3(0, 0, 0));
	float layer100 = getProbeLayer(gridCell + vec3(1, 0, 0));
	float layer010 = getProbeLayer(gridCell + vec3(0, 1, 0));
	float layer001 = getProbeLayer(gridCell + vec3(0, 0, 1));
	float layer110 = getProbeLayer(gridCell + vec3(1, 1, 0));
	float layer101 = getProbeLayer(gridCell + vec3(1, 0, 1));
	float layer011 = getProbeLayer(gridCell + vec3(0, 1, 1));
	float layer111 = getProbeLayer(gridCell + vec3(1, 1, 1));

    vec3 envcolor000 = textureLod(uReflectionProbeArray, vec4(R, layer000), roughness * maxLevel).rgb;// * probeFactor000;
	vec3 envcolor100 = textureLod(uReflectionProbeArray, vec4(R, layer100), roughness * maxLevel).rgb;// * probeFactor100;
	vec3 envcolor010 = textureLod(uReflectionProbeArray, vec4(R, layer010), roughness * maxLevel).rgb;// * probeFactor010;
	vec3 envcolor001 = textureLod(uReflectionProbeArray, vec4(R, layer001), roughness * maxLevel).rgb;// * probeFactor001;
	vec3 envcolor110 = textureLod(uReflectionProbeArray, vec4(R, layer110), roughness * maxLevel).rgb;// * probeFactor110;
	vec3 envcolor101 = textureLod(uReflectionProbeArray, vec4(R, layer101), roughness * maxLevel).rgb;// * probeFactor101;
	vec3 envcolor011 = textureLod(uReflectionProbeArray, vec4(R, layer011), roughness * maxLevel).rgb;// * probeFactor011;
	vec3 envcolor111 = textureLod(uReflectionProbeArray, vec4(R, layer111), roughness * maxLevel).rgb;// * probeFactor111;

	vec3 probePos000 = getProbePosition(gridCell + vec3(0, 0, 0));
	vec3 probePos100 = getProbePosition(gridCell + vec3(1, 0, 0));
	vec3 probePos010 = getProbePosition(gridCell + vec3(0, 1, 0));
	vec3 probePos001 = getProbePosition(gridCell + vec3(0, 0, 1));
	vec3 probePos110 = getProbePosition(gridCell + vec3(1, 1, 0));
	vec3 probePos101 = getProbePosition(gridCell + vec3(1, 0, 1));
	vec3 probePos011 = getProbePosition(gridCell + vec3(0, 1, 1));
	vec3 probePos111 = getProbePosition(gridCell + vec3(1, 1, 1));

	float probeVis000 = dot(probePos000 - worldPos, N) > 0.0 ? 1.0 : -1.0;
	float probeVis100 = dot(probePos100 - worldPos, N) > 0.0 ? 1.0 : -1.0;
	float probeVis010 = dot(probePos010 - worldPos, N) > 0.0 ? 1.0 : -1.0;
	float probeVis001 = dot(probePos001 - worldPos, N) > 0.0 ? 1.0 : -1.0;
	float probeVis110 = dot(probePos110 - worldPos, N) > 0.0 ? 1.0 : -1.0;
	float probeVis101 = dot(probePos011 - worldPos, N) > 0.0 ? 1.0 : -1.0;
	float probeVis011 = dot(probePos101 - worldPos, N) > 0.0 ? 1.0 : -1.0;
	float probeVis111 = dot(probePos111 - worldPos, N) > 0.0 ? 1.0 : -1.0;


	vec3 weights = fract(worldPos / uProbeGridCellSize);
	vec3 envcolor = mix(mix(mix(envcolor000, envcolor100, weights.x), mix(envcolor010, envcolor110, weights.x), weights.y),
		mix(mix(envcolor001, envcolor101, weights.x), mix(envcolor011, envcolor111, weights.x), weights.y), weights.z);
	*/


    return envcolor * (specularColor * envbrdf.x + envbrdf.y);
}
