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
	RTCDevice device = nullptr;
	RTCScene scene = nullptr;

	// Debugging
	int debugImageWidth;
	int debugImageHeight;
	std::vector<glm::vec3> debugImage;
	glow::SharedTexture2D debugTexture;
	const glow::camera::GenericCamera* debugCamera;
};