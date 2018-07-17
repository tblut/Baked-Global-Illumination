in vec2 aPosition;

out vec2 vTexCoord;

uniform vec2 uOffset;
uniform vec2 uScale;

void main() {
	vTexCoord = aPosition.xy;
	gl_Position = vec4((aPosition * uScale + uOffset) * 2 - 1, 0, 1);
}
