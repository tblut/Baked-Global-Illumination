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
	void render(RenderPipeline& pipeline) const;

	DirectionalLight& getSun();
	const DirectionalLight& getSun() const;

private:
	std::vector<Mesh> meshes;
	DirectionalLight sun;
};