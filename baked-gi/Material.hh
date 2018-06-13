#pragma once

#include <glow/fwd.hh>
#include <glm/glm.hpp>

struct Material {
	glow::SharedTexture2D colorMap;
	glow::SharedTexture2D roughnessMap;
	glow::SharedTexture2D normalMap;
	glow::SharedTexture2D lightMap;
	float roughness = 0.5f;
	float metallic = 0.0f;
	glm::vec3 baseColor = glm::vec3(1.0);
};