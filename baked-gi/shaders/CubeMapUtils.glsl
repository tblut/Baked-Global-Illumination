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
