#pragma once

#include "Image.hh"

#include <glm/glm.hpp>
#include <vector>

class Primitive {
public:
	// Node
	glm::mat4 transform;

	// Geometry
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> tangents;
	std::vector<glm::vec2> texCoords;
	std::vector<unsigned int> indices;
	GLenum mode;

	// Material
	SharedImage albedoMap;
	SharedImage normalMap;
	SharedImage roughnessMap;
	glm::vec3 baseColor = glm::vec3(1.0f);
	float roughness = 1.0f;
	float metallic = 1.0f;
};