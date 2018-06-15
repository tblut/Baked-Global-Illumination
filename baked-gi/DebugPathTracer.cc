#include "DebugPathTracer.hh"
#include "ColorUtils.hh"

#include <glow/objects/Texture2D.hh>
#include <glow/data/SurfaceData.hh>
#include <glow/data/TextureData.hh>
#include <random>

namespace {
	std::random_device randDevice;
	std::default_random_engine randEngine(randDevice());
	std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);
}

void DebugPathTracer::traceDebugImage() {
	std::vector<glm::vec3> colors(debugImage.size(), glm::vec3(0.0f));

	for (unsigned int k = 0; k < samplesPerPixel; ++k) {

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

void DebugPathTracer::saveDebugImageToFile(const std::string& path) const {
	std::vector<unsigned char> pixels(debugImageWidth * debugImageHeight * 3);
	for (int i = 0; i < debugImageWidth * debugImageHeight; ++i) {
		pixels[i * 3] = static_cast<unsigned char>(debugImage[i].x * 255.0f);
		pixels[i * 3 + 1] = static_cast<unsigned char>(debugImage[i].y * 255.0f);
		pixels[i * 3 + 2] = static_cast<unsigned char>(debugImage[i].z * 255.0f);
	}

	auto surface = std::make_shared<glow::SurfaceData>();
	auto dataPtr = pixels.data();
	surface->setData(std::vector<char>(dataPtr, dataPtr + debugImageWidth * debugImageHeight * 3));
	surface->setMipmapLevel(0);
	surface->setWidth(debugImageWidth);
	surface->setHeight(debugImageHeight);
	surface->setType(GL_UNSIGNED_BYTE);
	surface->setFormat(GL_RGB);

	auto tex = std::make_shared<glow::TextureData>();
	tex->setWidth(debugImageWidth);
	tex->setHeight(debugImageHeight);
	tex->addSurface(surface);
	tex->saveToFile(path);
}