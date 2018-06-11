#include "DebugPathTracer.hh"
#include "ColorUtils.hh"

#include <glow/objects/Texture2D.hh>
#include <glow/data/SurfaceData.hh>
#include <random>

namespace {
	std::random_device randDevice;
	std::default_random_engine randEngine(randDevice());
	std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);
}

void DebugPathTracer::traceDebugImage() {
	std::vector<glm::vec3> colors(debugImage.size(), glm::vec3(0.0f));

	for (int k = 0; k < samplesPerPixel; ++k) {

		#pragma omp parallel for
		for (int y = 0; y < debugImageHeight; ++y) {
			for (int x = 0; x < debugImageWidth; ++x) {
				float aspect = debugImageWidth / static_cast<float>(debugImageHeight);
				float fov = debugCamera->getHorizontalFieldOfView();
				float scale = std::tan(glm::radians(fov * 0.5f));
				float newX = x + uniformDist(randEngine) - 0.5f;
				float newY = y + uniformDist(randEngine) - 0.5f;
				float px = (2.0f * ((newX + 0.5f) / debugImageWidth) - 1.0f) * scale * aspect;
				float py = (1.0f - 2.0f * ((newY + 0.5f) / debugImageHeight)) * scale;

				glm::vec3 dir = glm::transpose(debugCamera->getViewMatrix()) * glm::vec4(px, py, -1, 0);
				colors[x + y * debugImageWidth] += trace(debugCamera->getPosition(), dir, glm::vec3(1.0f));
			}
		}
	}

	for (std::size_t i = 0; i < colors.size(); ++i) {
		debugImage[i] = colors[i] / static_cast<float>(samplesPerPixel);
		debugImage[i] = debugImage[i] / (debugImage[i] + glm::vec3(1.0f)); // Tone mapping
		debugImage[i] = linearToGamma(debugImage[i]); // Gamma correction
	}

	debugTexture->bind().setData(GL_RGB, debugImageWidth, debugImageHeight, debugImage);
}

void DebugPathTracer::attachDebugCamera(const glow::camera::GenericCamera& camera) {
	debugCamera = &camera;
}

void DebugPathTracer::setSamplesPerPixel(unsigned int sampleCount) {
	samplesPerPixel = sampleCount;
}

void DebugPathTracer::setDebugImageSize(int width, int height) {
	debugImageWidth = width;
	debugImageHeight = height;
	debugImage.resize(width * height);
	if (!debugTexture) {
		debugTexture = glow::Texture2D::create(width, height, GL_RGB);
		debugTexture->bind().setFilter(GL_LINEAR, GL_LINEAR);
	}
	else {
		debugTexture->bind().resize(width, height);
	}
}

glow::SharedTexture2D DebugPathTracer::getDebugTexture() const {
	return debugTexture;
}