#pragma once

#include <glm/glm.hpp>
#include <glow/fwd.hh>

struct ReflectionProbe {
    glm::vec3 position;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
	unsigned int layer; // The layer in the cube map array
};
