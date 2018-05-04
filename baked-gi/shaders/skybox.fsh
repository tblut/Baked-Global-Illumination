in vec3 vTexCoord;

out vec3 fColor;
out vec3 fBrightColor;

uniform samplerCube uSkybox;

void main() {
	fColor = texture(uSkybox, vTexCoord).rgb;

	// Extract brightness for bloom
	float brightness = dot(fColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        fBrightColor = fColor;
    }
	else {
        fBrightColor = vec3(0.0);
	}
}