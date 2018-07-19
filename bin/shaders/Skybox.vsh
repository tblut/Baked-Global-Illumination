in vec3 aPosition;

out vec3 vTexCoord;

uniform mat4 uView;
uniform mat4 uProj;

void main() {
	vTexCoord = aPosition;
	vec4 pos = uProj * uView * vec4(aPosition, 1.0);
	gl_Position = pos.xyww;
}