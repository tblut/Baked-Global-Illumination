#pragma once

#include "PathTracer.hh"
#include <string>

class DebugPathTracer : public PathTracer {
public:
	virtual ~DebugPathTracer() = default;

	void traceDebugImage();
	void attachDebugCamera(const glow::camera::GenericCamera& camera);
	void setSamplesPerPixel(unsigned int sampleCount);
	void setDebugImageSize(int width, int height);
	glow::SharedTexture2D getDebugTexture() const;
	void saveDebugImageToFile(const std::string& path) const;

private:
	int debugImageWidth;
	int debugImageHeight;
	std::vector<glm::vec3> debugImage;
	glow::SharedTexture2D debugTexture;
	const glow::camera::GenericCamera* debugCamera;
	unsigned int samplesPerPixel;
};