uniform vec3 uProbeGridDimensions;
uniform vec3 uProbeGridCellSize;
uniform sampler1DArray uProbeInfluenceTexture; // 0 = pos, 0.5 = min, 1 = max
uniform sampler3D uProbeVisibilityTexture; // The three probe layers used for the i-th voxel. Dim = w * h * d

vec3 getProbeGridCell(vec3 worldPos) {
	return floor(worldPos / uProbeGridCellSize);
}

float getProbeLayer(vec3 coord) {
	return coord.x + coord.y * uProbeGridDimensions.x + coord.z * uProbeGridDimensions.x * uProbeGridDimensions.y;
}

vec3 getProbePosition(float layer) {
	return texture(uProbeInfluenceTexture, vec2(0.01, layer)).xyz;
}

vec3 getProbeInfluenceBoxMin(float layer) {
	return texture(uProbeInfluenceTexture, vec2(0.5, layer)).xyz;
}

vec3 getProbeInfluenceBoxMax(float layer) {
	return texture(uProbeInfluenceTexture, vec2(0.99, layer)).xyz;
}

vec3 getProbeLayersForVoxel(vec3 coord) {
	vec3 texCoord = coord / uProbeGridDimensions;
	return texture(uProbeVisibilityTexture, texCoord).xyz;
}
