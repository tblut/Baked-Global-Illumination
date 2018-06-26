#include "PrecalcCommon.glsl"

uniform float uRoughness;
uniform samplerCube uEnvMap;

// See: https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
vec3 PrefilterEnvMap(float Roughness, vec3 R)
{
	vec3 N = R;
	vec3 V = R;

	vec3 color = vec3(0, 0, 0);
	float totalWeight = 0;

	const int samples = 1024;

	for (int i = 0; i < samples; ++i)
	{
		vec2 Xi = Hammersley(i, samples);
		vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
		vec3 L = 2 * dot(V, H) * H - V;

		float dotNL = max(0.0, dot(N, L));
		if (dotNL > 0)
		{
			color += textureLod(uEnvMap, L, 0).rgb * dotNL;
			totalWeight += dotNL;
		}
	}

	return color / totalWeight;
}

// Taken from glow-extras-material
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

uniform layout(binding = 0) writeonly imageCube uCube;

vec3 direction(float x, float y, uint l)
{
	// see ogl spec 8.13. CUBE MAP TEXTURE SELECTION
	switch (l) {
		// +x
	case 0: return vec3(+1, -y, -x);
		// -x
	case 1: return vec3(-1, -y, +x);
		// +y
	case 2: return vec3(+x, +1, +y);
		// -y
	case 3: return vec3(+x, -1, -y);
		// +z
	case 4: return vec3(+x, -y, +1);
		// -z
	case 5: return vec3(-x, -y, -1);
	}
	return vec3(0, 1, 0);
}

void main()
{
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;
	uint l = gl_GlobalInvocationID.z;
	ivec2 s = imageSize(uCube);

	if (x >= s.x || y >= s.y)
		return; // out of bounds

	float fx = (float(x) + .5) / float(s.x);
	float fy = (float(y) + .5) / float(s.y);

	vec3 dir = normalize(direction(fx * 2 - 1, fy * 2 - 1, l));

	vec3 color = PrefilterEnvMap(uRoughness, dir);

	imageStore(uCube, ivec3(x, y, l), vec4(color, 0));
}
