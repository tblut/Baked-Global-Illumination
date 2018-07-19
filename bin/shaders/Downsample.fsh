uniform sampler2DRect uColorBuffer;

out vec3 fColor;

void main() {
    fColor = texture(uColorBuffer, gl_FragCoord.xy * 2).rgb;
}