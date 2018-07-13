uniform vec3 uProbeGridDimensions;
uniform vec3 uProbeGridCellSize;
uniform sampler1DArray uProbePositionTexture;
uniform sampler1D uProbeVisibilityTexture; // The three probe layers used for the i-th voxel. Dim = w * h * d
uniform sampler1DArray uProbeAABBTexture; //layer = probe layer, 0 = min, 1 = max

vec3 getProbeGridCell(vec3 worldPos) {
	return floor(worldPos / uProbeGridCellSize);
}

float getProbeLayer(vec3 coord) {
	return coord.x + coord.y * uProbeGridDimensions.x + coord.z * uProbeGridDimensions.x * uProbeGridDimensions.y;
}

vec3 getProbePositionForCoord(vec3 coord) {
	return coord * uProbeGridCellSize;
}

vec3 getProbePositionForLayer(float layer) {
	return texture(uProbePositionTexture, vec2(0, layer)).xyz;
}

vec3 getProbeAABBMin(float layer) {
	return texture(uProbeAABBTexture, vec2(0, layer)).xyz;
}

vec3 getProbeAABBMax(float layer) {
	return texture(uProbeAABBTexture, vec2(1, layer)).xyz;
}

vec3 getProbeLayersForVoxel(vec3 coord) {
	float index = coord.x + coord.y * uProbeGridDimensions.x + coord.z * uProbeGridDimensions.x * uProbeGridDimensions.y;
	return texture(uProbeVisibilityTexture, index).xyz;
}
