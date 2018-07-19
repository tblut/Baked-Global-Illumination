in vec3 aPosition;
in vec3 aNormal;
#ifdef TEXTURE_MAPPING
in vec4 aTangent;
in vec2 aTexCoord;
#endif
in vec2 aLightMapTexCoord;

out vec3 vWorldPos;
out vec3 vNormal;
#ifdef TEXTURE_MAPPING
out vec4 vTangent;
out vec2 vTexCoord;
#endif
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
#ifdef TEXTURE_MAPPING
    vTangent = vec4(uNormalMat * aTangent.xyz, aTangent.w);
	vTexCoord = aTexCoord;
#endif
	vLightMapTexCoord = aLightMapTexCoord;
	vLightSpacePos = uLightMatrix * uModel * vec4(aPosition, 1.0);
	gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
}
