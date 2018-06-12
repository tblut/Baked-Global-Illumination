uniform sampler2DRect uHdrBuffer;
uniform sampler2DRect uBloomBuffer;

out vec3 fColor;

void main() {
    vec3 hdrColor = texelFetch(uHdrBuffer, ivec2(gl_FragCoord.xy)).rgb;
	vec3 bloomColor = texture(uBloomBuffer, gl_FragCoord.xy / 2).rgb;
	hdrColor += bloomColor * 0.6;

    // Reinhard tone mapping
    vec3 ldrColor = hdrColor / (hdrColor + vec3(1.0));

    // Gamma correction 
    ldrColor = pow(ldrColor, vec3(1.0 / 2.2));
  
    fColor = ldrColor;
}