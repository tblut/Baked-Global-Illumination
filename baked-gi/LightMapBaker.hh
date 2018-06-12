#pragma once

#include "Image.hh"

#include <glm/glm.hpp>
#include <vector>

class PathTracer;
class Primitive;

class LightMapBaker {
public:
	LightMapBaker(const PathTracer& pathTracer);

	SharedImage bakeLightMap(const Primitive& primitive, int width, int height);

private:
	const PathTracer* pathTracer;

};