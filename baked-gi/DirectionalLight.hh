#pragma once

#include <glm/glm.hpp>

struct DirectionalLight {
	glm::vec3 direction = glm::vec3(1.0f, -5.0f, -2.0f);
	glm::vec3 color = glm::vec3(1.0f, 0.9f, 0.8f);
	float power = 1.0f;
};