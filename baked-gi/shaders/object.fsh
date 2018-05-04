in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vTangent;
in vec2 vTexCoord;

out vec3 fColor;
out vec3 fBrightColor;

uniform vec3 uCamPos;
uniform vec3 uBaseColor;
uniform float uRoughness;
uniform float uMetallic;
uniform vec3 uAmbientColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;

uniform sampler2D uTextureColor;
uniform sampler2D uTextureNormal;

vec3 unpackNormalmap(vec3 rgb) {
  return vec3(rgb.rg * 2 - 1 - 1 / 256, rgb.b);
}

// DO NOT MULTIPLY BY COS THETA
vec3 shadingSpecularGGX(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0) {
    // see http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/
    vec3 H = normalize(V + L);

    float dotLH = max(dot(L, H), 0.0);
    float dotNH = max(dot(N, H), 0.0);
    float dotNL = max(dot(N, L), 0.0);
    float dotNV = max(dot(N, V), 0.0);

    float alpha = roughness * roughness;

    // D (GGX normal distribution)
    float alphaSqr = alpha * alpha;
    float denom = dotNH * dotNH * (alphaSqr - 1.0) + 1.0;
    float D = alphaSqr / (denom * denom);
    // no pi because BRDF -> lighting

    // F (Fresnel term)
    float F_a = 1.0;
    float F_b = pow(1.0 - dotLH, 5);
    vec3 F = mix(vec3(F_b), vec3(F_a), F0);

    // G (remapped hotness, see Unreal Shading)
    float k = (alpha + 2 * roughness + 1) / 8.0;
    float G = dotNL / (mix(dotNL, 1, k) * mix(dotNV, 1, k));
    // '* dotNV' - canceled by normalization

    // '/ dotLN' - canceled by lambert
    // '/ dotNV' - canceled by G
    return D * F * G / 4.0;
}

void main() {
	vec3 N = normalize(vNormal);
	vec3 T = normalize(vTangent);
	vec3 B = normalize(cross(T, N));
	mat3 TBN = mat3(T, B, N);

	// Texturing
	vec3 color = texture(uTextureColor, vTexCoord).rgb * uBaseColor;
    vec3 normal = unpackNormalmap(texture(uTextureNormal, vTexCoord).rgb);
    N = normalize(TBN * normal);

	// Shading
	vec3 V = normalize(uCamPos - vWorldPos);
	vec3 L = normalize(-uLightDir);
	vec3 diffuse = color * (1 - uMetallic);
    vec3 specular = mix(vec3(0.04), color, uMetallic);
    fColor = uAmbientColor * color + diffuse * max(dot(N, L), 0.0) + shadingSpecularGGX(N, V, L, max(0.01, uRoughness), specular);
	fColor = fColor * uLightColor;

	// Extract brightness for bloom
	float brightness = dot(fColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        fBrightColor = fColor;
    }
	else {
        fBrightColor = vec3(0.0);
	}
}