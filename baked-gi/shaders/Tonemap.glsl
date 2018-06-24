/*
* See: http://filmicworlds.com/blog/filmic-tonemapping-operators/
*/
uniform float uExposureAdjustment;

vec3 tonemapOperator(vec3 x) {
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 filmicTonemap(vec3 x) {
	float W = 11.2;
	vec3 whiteScale = 1.0 / tonemapOperator(vec3(W));
    return tonemapOperator(x * uExposureAdjustment) * whiteScale;
}
