in vec3 vTexCoord;

out vec3 fColor;
out vec3 fBrightColor;

uniform float uBloomPercentage;
uniform samplerCube uSkybox;

void main() {
	vec3 color = texture(uSkybox, vTexCoord).rgb;
	fColor = color * (1.0 - uBloomPercentage);
	fBrightColor = color * uBloomPercentage;
}