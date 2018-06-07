#include "PathTracer.hh"

#include <glow/objects/Texture2D.hh>
#include <glow/data/SurfaceData.hh>

#include <xmmintrin.h>

#if !defined(_MM_SET_DENORMALS_ZERO_MODE)
#define _MM_DENORMALS_ZERO_ON   (0x0040)
#define _MM_DENORMALS_ZERO_OFF  (0x0000)
#define _MM_DENORMALS_ZERO_MASK (0x0040)
#define _MM_SET_DENORMALS_ZERO_MODE(x) (_mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (x)))
#endif

PathTracer::PathTracer() {
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	device = rtcNewDevice("verbose=1");

	// Create the debug image and texture
	setDebugImageSize(512, 512);
}

PathTracer::~PathTracer() {
	rtcReleaseDevice(device);
}

void PathTracer::traceDebugImage() {
	for (int y = 0; y < debugImageHeight; ++y) {
		for (int x = 0; x < debugImageWidth; ++x) {
			debugImage[x + y * debugImageWidth] = { 1.0f, 1.0f, 0.0f };
		}
	}

	debugTexture->bind().setData(GL_RGB, debugImageWidth, debugImageHeight, debugImage);
}

void PathTracer::attachDebugCamera(const glow::camera::CameraBase& camera) {
	debugCamera = &camera;
}

void PathTracer::setDebugImageSize(int width, int height) {
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

glow::SharedTexture2D PathTracer::getDebugTexture() const {
	return debugTexture;
}