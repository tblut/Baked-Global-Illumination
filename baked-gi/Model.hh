#pragma once

#include "Material.hh"

#include <glow/fwd.hh>
#include <glow-extras/camera/CameraBase.hh>
#include <glow/objects/Program.hh>

#include <vector>
#include <string>
#include <unordered_map>

class RenderPipeline;

struct Mesh {
	glow::SharedVertexArray vao;
	Material material;
};

class Model {
public:
	void loadFromFile(const std::string& path, const std::string& texturesPath);
	void render(const glow::camera::CameraBase& camera, RenderPipeline& pipeline) const;

private:
	std::unordered_map<std::string, glow::SharedTexture2D> textures;
	std::vector<Mesh> meshes;
	glow::SharedTexture2D defaultNormalMap;
};