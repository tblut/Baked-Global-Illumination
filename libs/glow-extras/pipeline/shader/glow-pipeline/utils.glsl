uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

float wang_float(uint hash)
{
    return hash / float(0x7FFFFFFF) / 2.0;
}

vec3 sRGBtoLinear(vec3 color)
{
    return pow(color, vec3(2.2));
}
vec3 linearToSRGB(vec3 color)
{
    return pow(color, vec3(1.0 / 2.2));
}
