#pragma once

#include "Primitive.hh"

#include <embree3/rtcore.h>
#include <glow/fwd.hh>
#include <glow-extras/camera/GenericCamera.hh>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

class PathTracer {
public:
	PathTracer();
	~PathTracer();

	void buildScene(const std::vector<Primitive>& primitives);

	// Debugging
	void traceDebugImage();
	void attachDebugCamera(const glow::camera::GenericCamera& camera);
	void setDebugImageSize(int width, int height);
	glow::SharedTexture2D getDebugTexture() const;

private:
	struct Triangle {
		unsigned int v0;
		unsigned int v1;
		unsigned int v2;
	};

	struct Material {
		SharedImage albedoMap;
		SharedImage normalMap;
		SharedImage roughnessMap;
		glm::vec3 baseColor = glm::vec3(1.0f);
		float roughness = 0.8f;
		float metallic = 0.0f;
	};

	RTCDevice device = nullptr;
	RTCScene scene = nullptr;

	std::unordered_map<unsigned int, Material> materials;

	// Debugging
	int debugImageWidth;
	int debugImageHeight;
	std::vector<glm::vec3> debugImage;
	glow::SharedTexture2D debugTexture;
	const glow::camera::GenericCamera* debugCamera;
};