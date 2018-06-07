#pragma once

#include "Material.hh"
#include "Mesh.hh"
#include "RenderPipeline.hh"
#include "DirectionalLight.hh"

#include <glow/fwd.hh>
#include <string>
#include <vector>

class Scene {
public:
	void loadFromGltf(const std::string& path);
	void render(const glow::camera::CameraBase& camera, RenderPipeline& pipeline) const;

private:
	std::vector<Mesh> meshes;
	DirectionalLight sun;
};