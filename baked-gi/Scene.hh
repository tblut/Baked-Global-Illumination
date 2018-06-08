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
	void loadFromGltf(const std::string& path, bool makeRealtimeObjects = true);
	void render(RenderPipeline& pipeline) const;

	void buildPathTracerScene(PathTracer& pathTracer) const;

	DirectionalLight& getSun();
	const DirectionalLight& getSun() const;
	
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