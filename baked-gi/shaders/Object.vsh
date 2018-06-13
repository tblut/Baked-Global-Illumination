in vec3 aPosition;
in vec3 aNormal;
in vec4 aTangent;
in vec2 aTexCoord;
in vec2 aLightMapTexCoord;

out vec3 vWorldPos;
out vec3 vNormal;
out vec4 vTangent;
out vec2 vTexCoord;
out vec2 vLightMapTexCoord;

uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;
uniform mat3 uNormalMat;

void main() {
	vWorldPos = vec3(uModel * vec4(aPosition, 1.0));
	vNormal = uNormalMat * aNormal;
	vTangent = vec4(uNormalMat * aTangent.xyz, aTangent.w);
	vTexCoord = aTexCoord;
	vLightMapTexCoord = aLightMapTexCoord;
	gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
}