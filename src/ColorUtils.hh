#pragma once

#include <glm/glm.hpp>
#include <cmath>

inline glm::vec3 gammaToLinear(const glm::vec3& v) {
	return { std::pow(v.x, 2.2f), std::pow(v.y, 2.2f) , std::pow(v.z, 2.2f) };
}

inline glm::vec3 linearToGamma(const glm::vec3& v) {
	return { std::pow(v.x, 1.0f / 2.2f), std::pow(v.y, 1.0f / 2.2f) , std::pow(v.z, 1.0f / 2.2f) };
}