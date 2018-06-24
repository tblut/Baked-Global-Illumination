uniform samplerCube uEnvMap;

in vec3 vTexCoord;

out vec3 fColor;

void main() {
    fColor = texture(uEnvMap, vTexCoord).rgb;
}