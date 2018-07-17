#pragma once

#include "Image.hh"

#include <glm/glm.hpp>
#include <vector>
#include <functional>

class PathTracer;
class Primitive;

class IlluminationBaker {
public:
	IlluminationBaker(const PathTracer& pathTracer);

	SharedImage bakeIrradiance(const Primitive& primitive, int width, int height, int samplesPerTexel);
	SharedImage bakeAmbientOcclusion(const Primitive& primitive, int width, int height, int samplesPerTexel, float maxDistance);

private:
	using BakeOperator = std::function<glm::vec3(glm::vec3, glm::vec3)>;

	std::vector<glm::vec3> bake(const Primitive& primitive, int width, int height, int samplesPerTexel, const BakeOperator& op);

	const PathTracer* pathTracer;
};
