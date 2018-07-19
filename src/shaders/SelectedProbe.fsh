uniform vec3 uColor;
uniform vec3 uLightDir;

in vec3 vNormal;

out vec3 fColor;
out vec3 fBrightColor;

void main() {
    fColor = uColor * max(0.0, dot(normalize(vNormal), uLightDir));
    fBrightColor = vec3(0.0);
}
