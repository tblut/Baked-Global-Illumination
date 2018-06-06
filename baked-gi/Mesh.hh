#pragma once

#include "Material.hh"

#include <glow/fwd.hh>

struct Mesh {
	glow::SharedVertexArray vao;
	Material material;
};