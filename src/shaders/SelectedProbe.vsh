in vec3 aPosition;

out vec3 vNormal;

uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;

void main() {
	vNormal = aPosition;
	gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
}
