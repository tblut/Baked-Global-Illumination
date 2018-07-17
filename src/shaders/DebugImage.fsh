uniform sampler2D uDebugImage;

in vec2 vTexCoord;

out vec3 fColor;

void main() {
    fColor = texture(uDebugImage, vec2(vTexCoord.x, 1.0 - vTexCoord.y)).rgb;
}