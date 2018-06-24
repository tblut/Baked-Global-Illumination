uniform samplerCube uEnvMap;
uniform float uMipLevel;

in vec3 vTexCoord;

out vec3 fColor;

void main() {
    fColor = textureLod(uEnvMap, vTexCoord, uMipLevel).rgb;
}