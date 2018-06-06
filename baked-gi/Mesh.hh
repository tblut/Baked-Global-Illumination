#pragma once

#include "Material.hh"

#include <glow/fwd.hh>
#include <glm/glm.hpp>

struct Mesh {
	glow::SharedVertexArray vao;
	Material material;
	glm::mat4 transform;
};