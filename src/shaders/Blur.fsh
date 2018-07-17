uniform sampler2DRect uColorBuffer;
uniform float uOffset;

out vec3 fColor;

void main() {
	vec2 offset = vec2(0.5 + uOffset, 0.5 + uOffset);
	vec3 color = vec3(0.0);
	color += texture(uColorBuffer, gl_FragCoord.xy + vec2(offset.x, offset.y)).rgb;
	color += texture(uColorBuffer, gl_FragCoord.xy + vec2(-offset.x, offset.y)).rgb;
	color += texture(uColorBuffer, gl_FragCoord.xy + vec2(offset.x, -offset.y)).rgb;
	color += texture(uColorBuffer, gl_FragCoord.xy + vec2(-offset.x, -offset.y)).rgb;

    fColor = color / 4.0;
}