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

bool isInBox(vec3 worldPos, vec3 aabbMin, vec3 aabbMax) {
	return worldPos.x > aabbMin.x && worldPos.x < aabbMax.x
		&& worldPos.y > aabbMin.y && worldPos.y < aabbMax.y
		&& worldPos.z > aabbMin.z && worldPos.z < aabbMax.z;
}

bool isInInnerBox(vec3 worldPos, vec3 aabbMin, vec3 aabbMax) {
	vec3 halfSize = (aabbMax - aabbMin) * 0.5;
	vec3 innerMin = aabbMin + halfSize * 0.5;
	vec3 innerMax = aabbMax - halfSize * 0.5;
	return worldPos.x > innerMin.x && worldPos.x < innerMax.x
		&& worldPos.y > innerMin.y && worldPos.y < innerMax.y
		&& worldPos.z > innerMin.z && worldPos.z < innerMax.z;
}


float getInfluenceWeight(vec3 worldPos, vec3 aabbMin, vec3 aabbMax) {
	vec3 localPos = worldPos - aabbMin;
	vec3 localDir = vec3(abs(localPos.x), abs(localPos.y), abs(localPos.z));

	vec3 innerHalfSize = (aabbMax - aabbMin) * 0.25;
	vec3 innerSize = (aabbMax - innerHalfSize) - (aabbMin + innerHalfSize);
	vec3 outerSize = aabbMax - aabbMin;

	localDir = (localDir - innerSize) / (outerSize - innerSize);
	return max(localDir.x, max(localDir.y, localDir.z));
}

float getBlendMapFactors1(vec3 worldPos, vec3 aabbMin0, vec3 aabbMax0) {
    // First calc sum of NDF and InvDNF to normalize value
    float sumNDF = 0.0;
    float invSumNDF = 0.0;
    float sumBlendFactor = 0.0;

	float ndf0 = getInfluenceWeight(worldPos, aabbMin0, aabbMax0);

	sumNDF += ndf0;

	invSumNDF += (1.0 - ndf0);

	float blendFactor;
	blendFactor = (1.0 - (ndf0 / sumNDF)) / (2 - 1);
	blendFactor *= ((1.0 - ndf0) / invSumNDF);

	sumBlendFactor += blendFactor;

    if (sumBlendFactor == 0.0) {
        sumBlendFactor = 1.0;
    }

    float constVal = 1.0 / sumBlendFactor;
	blendFactor *= constVal;

	return blendFactor;
}

vec2 getBlendMapFactors2(vec3 worldPos, vec3 aabbMin0, vec3 aabbMax0, vec3 aabbMin1, vec3 aabbMax1) {
    // First calc sum of NDF and InvDNF to normalize value
    float sumNDF = 0.0;
    float invSumNDF = 0.0;
    float sumBlendFactor = 0.0;

	float ndf0 = getInfluenceWeight(worldPos, aabbMin0, aabbMax0);
	float ndf1 = getInfluenceWeight(worldPos, aabbMin1, aabbMax1);

	sumNDF += ndf0;
	sumNDF += ndf1;

	invSumNDF += (1.0 - ndf0);
	invSumNDF += (1.0 - ndf1);

	vec2 blendFactors;
	blendFactors.x = (1.0 - (ndf0 / sumNDF)) / (2 - 1);
	blendFactors.x *= ((1.0 - ndf0) / invSumNDF);
	blendFactors.y = (1.0 - (ndf1 / sumNDF)) / (2 - 1);
	blendFactors.y *= ((1.0 - ndf1) / invSumNDF);

	sumBlendFactor += blendFactors.x;
	sumBlendFactor += blendFactors.y;

    if (sumBlendFactor == 0.0) {
        sumBlendFactor = 1.0;
    }

    float constVal = 1.0 / sumBlendFactor;
	blendFactors *= constVal;

	return blendFactors;
}

vec3 getBlendMapFactors3(vec3 worldPos, vec3 aabbMin0, vec3 aabbMax0, vec3 aabbMin1, vec3 aabbMax1, vec3 aabbMin2, vec3 aabbMax2) {
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
