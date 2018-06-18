in vec3 aPosition;
in vec3 aNormal;
in vec2 aLightMapTexCoord;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vLightMapTexCoord;
out vec4 vLightSpacePos;

uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;
uniform mat3 uNormalMat;
uniform mat4 uLightMatrix;

void main() {
	vWorldPos = vec3(uModel * vec4(aPosition, 1.0));
	vNormal = uNormalMat * aNormal;
	vLightMapTexCoord = aLightMapTexCoord;
	vLightSpacePos = uLightMatrix * uModel * vec4(aPosition, 1.0);
	gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
}