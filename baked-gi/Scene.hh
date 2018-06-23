#pragma once

#include "Material.hh"
#include "Mesh.hh"
#include "RenderPipeline.hh"
#include "DirectionalLight.hh"
#include "Primitive.hh"
#include "Image.hh"

#include <glow/fwd.hh>
#include <string>
#include <vector>

class PathTracer;

class Scene {
public:
	void loadFromGltf(const std::string& path);
	void render(RenderPipeline& pipeline) const;

	void buildRealtimeObjects(const std::string& lightMapPath = "");
	void buildPathTracerScene(PathTracer& pathTracer) const;

	DirectionalLight& getSun();
	const DirectionalLight& getSun() const;
	const std::vector<Primitive>& getPrimitives() const;
	const std::vector<Mesh>& getMeshes() const;
	
private:
	// Common
	DirectionalLight sun;

	// Offline rendering
	std::vector<SharedImage> images;
	std::vector<Primitive> primitives;

	// Realtime rendering
	std::vector<glow::SharedTexture2D> textures;
	std::vector<Mesh> meshes;
};
