in vec3 aPosition;
in vec3 aNormal;

out vec3 vWorldPos;
out vec3 vNormal;

uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;

void main() {
	vWorldPos = vec3(uModel * vec4(aPosition, 1.0));
	vNormal = mat3(uModel) * aNormal;
	gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
}