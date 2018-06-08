#include "PathTracer.hh"

#include <glow/objects/Texture2D.hh>
#include <glow/data/SurfaceData.hh>

#include <xmmintrin.h>
#include <limits>

#if !defined(_MM_SET_DENORMALS_ZERO_MODE)
#define _MM_DENORMALS_ZERO_ON   (0x0040)
#define _MM_DENORMALS_ZERO_OFF  (0x0000)
#define _MM_DENORMALS_ZERO_MASK (0x0040)
#define _MM_SET_DENORMALS_ZERO_MODE(x) (_mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (x)))
#endif

namespace {
	struct Ray : public RTCRay {
		Ray(const glm::vec3& origin, const glm::vec3& dir, float tnear, float tfar) {
			this->org_x = origin.x;
			this->org_y = origin.y;
			this->org_z = origin.z;
			this->dir_x = dir.x;
			this->dir_y = dir.y;
			this->dir_z = dir.z;
			this->tnear = tnear;
			this->tfar = tfar;
			this->time = 0.0f;
		}
	};

	struct Hit : public RTCHit {
		Hit() {
			geomID = RTC_INVALID_GEOMETRY_ID;
		}
	};
}

PathTracer::PathTracer() {
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	device = rtcNewDevice("verbose=1");
}

PathTracer::~PathTracer() {
	if (scene) {
		rtcReleaseScene(scene);
	}
	if (device) {
		rtcReleaseDevice(device);
	}
}

void PathTracer::buildScene(const std::vector<Primitive>& primitives) {
	if (scene) {
		rtcReleaseScene(scene);
	}
	scene = rtcNewScene(device);
	
	for (const auto& primitive : primitives) {
		RTCGeometry mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

		auto indexBuffer = static_cast<unsigned int*>(rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX,
			0, RTC_FORMAT_UINT3, sizeof(unsigned int) * 3, primitive.indices.size()));
		std::memcpy(indexBuffer, primitive.indices.data(), primitive.indices.size() * sizeof(unsigned int));

		auto vertexBuffer = static_cast<glm::vec3*>(rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX,
			0, RTC_FORMAT_FLOAT3, sizeof(glm::vec3), primitive.positions.size()));
		std::memcpy(vertexBuffer, primitive.positions.data(), primitive.positions.size() * sizeof(glm::vec3));

		// Bake the transform into the positions
		for (std::size_t i = 0; i < primitive.positions.size(); ++i) {
			vertexBuffer[i] = glm::vec3(primitive.transform * glm::vec4(vertexBuffer[i], 1.0f));
		}

		rtcCommitGeometry(mesh);
		rtcAttachGeometry(scene, mesh);
		rtcReleaseGeometry(mesh);
	}

	rtcCommitScene(scene);
}

void PathTracer::traceDebugImage() {
	for (int y = 0; y < debugImageHeight; ++y) {
		for (int x = 0; x < debugImageWidth; ++x) {

			RTCIntersectContext context;
			rtcInitIntersectContext(&context);

			float aspect = debugImageWidth / static_cast<float>(debugImageHeight);
			float fov = debugCamera->getHorizontalFieldOfView();
			float scale = std::tan(glm::radians(fov * 0.5f));
			float px = (2.0f * ((x + 0.5f) / debugImageWidth) - 1.0f) * scale * aspect;
			float py = (1.0f - 2.0f * ((y + 0.5f) / debugImageHeight)) * scale;

			glm::vec3 dir = glm::transpose(debugCamera->getViewMatrix()) * glm::vec4(px, py, -1, 0);
			Ray ray(debugCamera->getPosition(), dir, 0.0f, std::numeric_limits<float>::infinity());
			RTCRayHit rayhit = { ray, Hit() };

			rtcIntersect1(scene, &context, &rayhit);
			if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
				debugImage[x + y * debugImageWidth] = { (rayhit.hit.Ng_x + 1) / 2, (rayhit.hit.Ng_y + 1) / 2, (rayhit.hit.Ng_z + 1) / 2 };
			}
			else {
				debugImage[x + y * debugImageWidth] = { 0.0f, 1.0f, 0.0f };
			}
		}
	}

	debugTexture->bind().setData(GL_RGB, debugImageWidth, debugImageHeight, debugImage);
}

void PathTracer::attachDebugCamera(const glow::camera::GenericCamera& camera) {
	debugCamera = &camera;
}

void PathTracer::setDebugImageSize(int width, int height) {
	glow::info() << width << "  " << height;

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