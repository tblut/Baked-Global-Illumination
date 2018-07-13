vec3 parallaxCorrectedReflection(vec3 R, vec3 objectPos, vec3 cubeMapPos, vec3 aabbMin, vec3 aabbMax) {
    vec3 firstPlaneIntersect = (aabbMax - objectPos) / R;
    vec3 secondPlaneIntersect = (aabbMin - objectPos) / R;
    
    // Get the furthest of these intersections along the ray
    // (Ok because x/0 give +inf and -x/0 give –inf )
    vec3 furthestPlane = max(firstPlaneIntersect, secondPlaneIntersect);
    
    // Find the closest far intersection
    float dist = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);

    // Get the intersection position
    vec3 intersectionPos = objectPos + R * dist;
    
    // Get corrected reflection
    return intersectionPos - cubeMapPos;
}

bool isInInnerBox(vec3 worldPos, vec3 aabbMin, vec3 aabbMax) {
	vec3 halfSize = (aabbMax - aabbMin) * 0.5;
	vec3 innerMin = aabbMin + halfSize * 0.5;
	vec3 innerMax = aabbMax - halfSize * 0.5;
	return worldPos.x > innerMin.x && worldPos.x < innerMax.x
		&& worldPos.y > innerMin.y && worldPos.y < innerMax.y
		&& worldPos.z > innerMin.z && worldPos.z < innerMax.z;
}
/*
float getInfluenceWeight(vec3 worldPos, vec3 aabbMin, vec3 aabbMax) {
	vec3 localPos = vec3(0);
	vec3 localDir = vec3(abs())
    Vector LocalDir = Vector(Abs(LocalPosition.X), Abs(LocalPosition.Y), Abs(LocalPosition.Z));
    LocalDir = (LocalDir - BoxInnerRange) / (BoxOuterRange - BoxInnerRange);
    // Take max of all axis
    NDF = LocalDir.GetMax();
}

void GetBlendMapFactor(int Num, CubemapInfluenceVolume* InfluenceVolume, float* BlendFactor)
{
    // First calc sum of NDF and InvDNF to normalize value
    float SumNDF            = 0.0f;
    float InvSumNDF         = 0.0f;
    float SumBlendFactor    = 0.0f;
    // The algorithm is as follow
    // Primitive have a normalized distance function which is 0 at center and 1 at boundary
    // When blending multiple primitive, we want the following constraint to be respect:
    // A - 100% (full weight) at center of primitive whatever the number of primitive overlapping
    // B - 0% (zero weight) at boundary of primitive whatever the number of primitive overlapping
    // For this we calc two weight and modulate them.
    // Weight0 is calc with NDF and allow to respect constraint B
    // Weight1 is calc with inverse NDF, which is (1 - NDF) and allow to respect constraint A
    // What enforce the constraint is the special case of 0 which once multiply by another value is 0.
    // For Weight 0, the 0 will enforce that boundary is always at 0%, but center will not always be 100%
    // For Weight 1, the 0 will enforce that center is always at 100%, but boundary will not always be 0%
    // Modulate weight0 and weight1 then renormalizing will allow to respects A and B at the same time.
    // The in between is not linear but give a pleasant result.
    // In practice the algorithm fail to avoid popping when leaving inner range of a primitive
    // which is include in at least 2 other primitives.
    // As this is a rare case, we do with it.
    for (INT i = 0; i < Num; ++i)
    {
        SumNDF       += InfluenceVolume(i).NDF;
        InvSumNDF    += (1.0f - InfluenceVolume(i).NDF);
    }

    // Weight0 = normalized NDF, inverted to have 1 at center, 0 at boundary.
    // And as we invert, we need to divide by Num-1 to stay normalized (else sum is > 1). 
    // respect constraint B.
    // Weight1 = normalized inverted NDF, so we have 1 at center, 0 at boundary
    // and respect constraint A.
    for (INT i = 0; i < Num; ++i)
    {
        BlendFactor[i] = (1.0f - (InfluenceVolume(i).NDF / SumNDF)) / (Num - 1);
        BlendFactor[i] *= ((1.0f - InfluenceVolume(i).NDF) / InvSumNDF);
        SumBlendFactor += BlendFactor[i];
    }

    // Normalize BlendFactor
    if (SumBlendFactor == 0.0f) // Possible with custom weight
    {
        SumBlendFactor = 1.0f;
    }

    float ConstVal = 1.0f / SumBlendFactor;
    for (int i = 0; i < Num; ++i)
    {
        BlendFactor[i] *= ConstVal;
    }
}

// Main code
for (int i = 0; i < NumPrimitive; ++i)
 {
     if (In inner range)
         EarlyOut;

     if (In outer range)
        SelectedInfluenceVolumes.Add(InfluenceVolumes.GetInfluenceWeights(LocationPOI));
 }

SelectedInfluenceVolumes.Sort();
GetBlendMapFactor(SelectedInfluenceVolumes.Num(), SelectedInfluenceVolumes, outBlendFactor)
*/