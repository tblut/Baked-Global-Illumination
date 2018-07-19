uniform samplerCube uEnvMap;
uniform float uMipLevel;

in vec3 vTexCoord;

out vec3 fColor;
out vec3 fBrightColor;

void main() {
    fColor = textureLod(uEnvMap, vTexCoord, uMipLevel).rgb;
    fBrightColor = vec3(0.0);
}
