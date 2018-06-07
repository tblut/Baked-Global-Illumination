#pragma once

#include <embree3/rtcore.h>
#include <glow/fwd.hh>
#include <glow-extras/camera/CameraBase.hh>
#include <glm/glm.hpp>
#include <vector>

class PathTracer {
public:
	PathTracer();
	~PathTracer();

	// Debugging
	void traceDebugImage();
	void attachDebugCamera(const glow::camera::CameraBase& camera);
	void setDebugImageSize(int width, int height);
	glow::SharedTexture2D getDebugTexture() const;

private:
	RTCDevice device;

	// Debugging
	int debugImageWidth;
	int debugImageHeight;
	std::vector<glm::vec3> debugImage;
	glow::SharedTexture2D debugTexture;
	const glow::camera::CameraBase* debugCamera;
};