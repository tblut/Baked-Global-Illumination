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

	SharedImage bakeIrradiance(const Primitive& primitive, int width, int height, int samplesPerTexel) const;
	SharedImage bakeAmbientOcclusion(const Primitive& primitive, int width, int height, int samplesPerTexel, float maxDistance) const;

private:
	using BakeOperator = std::function<glm::vec3(glm::vec3, glm::vec3)>;

	std::vector<glm::vec3> bake(const Primitive& primitive, int width, int height, int samplesPerTexel, const BakeOperator& op) const;
	void fillIllegalTexels(const Primitive& primitive, int width, int height, std::vector<glm::vec3>& values) const;
	glm::ivec2 findClosestLegalTexel(int x, int y, int width, int height, const std::vector<bool>& illegalMap) const;

	const PathTracer* pathTracer;
};
