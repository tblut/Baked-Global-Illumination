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
    return normalize(intersectionPos - cubeMapPos);
}

float getInfluenceWeight(vec3 worldPos, vec3 aabbMin, vec3 aabbMax) {
	vec3 halfSize = (aabbMax - aabbMin) * 0.5;
	vec3 center = aabbMin + halfSize;
	vec3 localPos = worldPos - center;
	vec3 localDir = vec3(abs(localPos.x), abs(localPos.y), abs(localPos.z));
	localDir = localDir / halfSize;
	float ndf = max(localDir.x, max(localDir.y, localDir.z));
	return min(ndf, 1.0);
}

vec3 getBlendMapFactors(vec3 worldPos, vec3 aabbMin0, vec3 aabbMax0, vec3 aabbMin1, vec3 aabbMax1, vec3 aabbMin2, vec3 aabbMax2) {
    // First calc sum of NDF and InvDNF to normalize value
    float sumNDF = 0.0;
    float invSumNDF = 0.0;
    float sumBlendFactor = 0.0;

	float ndf0 = getInfluenceWeight(worldPos, aabbMin0, aabbMax0);
	float ndf1 = getInfluenceWeight(worldPos, aabbMin1, aabbMax1);
	float ndf2 = getInfluenceWeight(worldPos, aabbMin2, aabbMax2);

	sumNDF += ndf0;
	sumNDF += ndf1;
	sumNDF += ndf2;

	invSumNDF += (1.0 - ndf0);
	invSumNDF += (1.0 - ndf1);
	invSumNDF += (1.0 - ndf2);
	invSumNDF = max(0.01, invSumNDF);

	vec3 blendFactors;
	blendFactors.x = (1.0 - (ndf0 / sumNDF)) / (3 - 1);
	blendFactors.x *= ((1.0 - ndf0) / invSumNDF);
	blendFactors.y = (1.0 - (ndf1 / sumNDF)) / (3 - 1);
	blendFactors.y *= ((1.0 - ndf1) / invSumNDF);
	blendFactors.z = (1.0 - (ndf2 / sumNDF)) / (3 - 1);
	blendFactors.z *= ((1.0 - ndf2) / invSumNDF);

	sumBlendFactor += blendFactors.x;
	sumBlendFactor += blendFactors.y;
	sumBlendFactor += blendFactors.z;

    if (sumBlendFactor == 0.0) {
        sumBlendFactor = 1.0;
    }

    float constVal = 1.0 / sumBlendFactor;
	blendFactors *= constVal;

	return blendFactors;
}
