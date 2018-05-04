in vec3 aPosition;
in vec3 aNormal;
in vec3 aTangent;
in vec2 aTexCoord;

out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vTangent;
out vec2 vTexCoord;

uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;

void main() {
	vWorldPos = vec3(uModel * vec4(aPosition, 1.0));
	vNormal = mat3(uModel) * aNormal;
	vTangent = mat3(uModel) * aTangent;
	vTexCoord = aTexCoord;
	gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
}