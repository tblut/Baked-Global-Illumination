uniform sampler2D uDebugImage;
uniform vec2 uViewportSize;

out vec3 fColor;

void main() {
	vec2 texCoord = gl_FragCoord.xy / uViewportSize * 2;
	texCoord.y = 1 - texCoord.y;
    fColor = texture(uDebugImage, texCoord).rgb;
}