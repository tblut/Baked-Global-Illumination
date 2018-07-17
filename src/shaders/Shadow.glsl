uniform sampler2DRectShadow uTextureShadow;
uniform vec2 uShadowMapSize;
uniform float uShadowOffset;

float calcShadowFactor(vec4 lightSpacePos) {
	vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
	projCoords.xy *= uShadowMapSize;
	projCoords.z -= uShadowOffset;

	float shadowFactor = 0.0;
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(-1.5, -1.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(-0.5, -1.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(0.5, -1.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(1.5, -1.5), projCoords.z));

	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(-1.5, -0.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(-0.5, -0.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(0.5, -0.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(1.5, -0.5), projCoords.z));

	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(-1.5, 0.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(-0.5, 0.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(0.5, 0.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(1.5, 0.5), projCoords.z));

	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(-1.5, 1.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(-0.5, 1.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(0.5, 1.5), projCoords.z));
	shadowFactor += texture(uTextureShadow, vec3(projCoords.xy + vec2(1.5, 1.5), projCoords.z));

	return shadowFactor * 0.25;
}