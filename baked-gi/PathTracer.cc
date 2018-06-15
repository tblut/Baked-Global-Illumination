#include "PathTracer.hh"
#include "ColorUtils.hh"

#include <glow/objects/Texture2D.hh>
#include <glow/data/SurfaceData.hh>

#include <xmmintrin.h>
#include <limits>
#include <cmath>
#include <algorithm>
#include <random>
#include <cassert>

#if !defined(_MM_SET_DENORMALS_ZERO_MODE)
#define _MM_DENORMALS_ZERO_ON   (0x0040)
#define _MM_DENORMALS_ZERO_OFF  (0x0000)
#define _MM_DENORMALS_ZERO_MASK (0x0040)
#define _MM_SET_DENORMALS_ZERO_MODE(x) (_mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (x)))
#endif

namespace {
	struct alignas(16) Vec3A {
		float x;
		float y;
		float z;
		float w;
	};

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

	std::random_device randDevice;
	std::default_random_engine randEngine(randDevice());
	std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);

	void makeCoordinateSystem(const glm::vec3& normal, glm::vec3& xAxis, glm::vec3& yAxis) {
		xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		if (std::abs(1.0f - normal.x) < 1.0e-8f) {
			xAxis = glm::vec3(0.0f, 0.0f, -1.0f);
		}
		else if (std::abs(1.0f + normal.x) < 1.0e-8f) {
			xAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		}

		yAxis = glm::normalize(glm::cross(normal, xAxis));
		xAxis = glm::normalize(glm::cross(yAxis, normal));
	}

	glm::vec3 sampleCosineHemisphere(const glm::vec3& normal) {
		float u1 = uniformDist(randEngine);
		float u2 = uniformDist(randEngine);
		
		float r = std::sqrt(u1);
		float phi = 2.0f * glm::pi<float>() * u2;

		float x = r * std::sin(phi);
		float y = r * std::cos(phi);
		float z = std::sqrt(1.0f - x * x - y * y);

		glm::vec3 xAxis;
		glm::vec3 yAxis;
		makeCoordinateSystem(normal, xAxis, yAxis);

		return glm::normalize(x * xAxis + y * yAxis + z * normal);
	}

	float pdfCosineHemisphere(const glm::vec3& normal, const glm::vec3& wi) {
		return glm::dot(normal, wi) / glm::pi<float>();
	}

	glm::vec3 brdfLambert(const glm::vec3& diffuse) {
		return diffuse / glm::pi<float>();
	}

	glm::vec3 sampleGGX(const glm::vec3& direction, float roughness) {
		float u1 = uniformDist(randEngine);
		float u2 = uniformDist(randEngine);

		float alpha = roughness * roughness;
		float phi = 2.0f * glm::pi<float>() * u1;
		float theta = std::acos(std::sqrt((1.0f - u2) / ((alpha * alpha - 1.0f) * u2 + 1.0f)));

		float x = std::sin(theta) * std::cos(phi);
		float y = std::sin(theta) * std::sin(phi);
		float z = std::cos(theta);

		glm::vec3 xAxis;
		glm::vec3 yAxis;
		makeCoordinateSystem(direction, xAxis, yAxis);

		return glm::normalize(x * xAxis + y * yAxis + z * direction);
	}

	float pdfGGX(const glm::vec3& halfVector, const glm::vec3& wi, float roughness) {
		float alpha = roughness * roughness;
		float alphaSq = alpha * alpha;
		float cosTheta = std::max(0.0f, glm::dot(halfVector, wi));
		float denom = (alphaSq - 1.0f) * cosTheta * cosTheta + 1.0f;
		return alphaSq * cosTheta / (glm::pi<float>() * denom * denom);
	}

	glm::vec3 brdfCookTorrenceGGX(const glm::vec3& N, const glm::vec3& V, const glm::vec3& L, float roughness, const glm::vec3& F0) {
		float dotNV = std::max(0.0f, glm::dot(N, V));
		float dotNL = std::max(0.0f, glm::dot(N, L));
		if (dotNV < 1e-8f || dotNL < 1e-8f) {
			return glm::vec3(0.0f);
		}

		glm::vec3 H = glm::normalize(V + L);
		if (std::abs(H.x) < 1e-8f && std::abs(H.y) < 1e-8f && std::abs(H.z) < 1e-8f) {
			return glm::vec3(0.0f);
		}

		float dotNH = std::max(0.0f, glm::dot(N, H));
		float dotVH = std::max(0.0f, glm::dot(V, H));

		float alpha = roughness * roughness;
		float alphaSq = alpha * alpha;
		float denom = dotNH * dotNH * (alphaSq - 1.0f) + 1.0f;
		float D = alphaSq / (glm::pi<float>() * denom * denom);

		float k = alpha / 2.0f;
		float G_l = dotNL / (dotNL * (1.0f - k) + k);
		float G_v = dotNV / (dotNV * (1.0f - k) + k);
		float G = G_l * G_v;

		glm::vec3 F = F0 + (glm::vec3(1.0f) - F0) * std::pow(1.0f - dotVH, 5.0f);

		return D * G * F / (4.0f * dotNL * dotNV);
	}
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
	rtcSetSceneFlags(scene, RTCSceneFlags::RTC_SCENE_FLAG_ROBUST);
	rtcSetSceneBuildQuality(scene, RTCBuildQuality::RTC_BUILD_QUALITY_HIGH);
	
	for (const auto& primitive : primitives) {
		RTCGeometry mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
		rtcSetGeometryBuildQuality(mesh, RTCBuildQuality::RTC_BUILD_QUALITY_HIGH);
		rtcSetGeometryVertexAttributeCount(mesh, 3);
		
		auto indexBuffer = static_cast<unsigned int*>(rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX,
			0, RTC_FORMAT_UINT3, sizeof(unsigned int) * 3, primitive.indices.size() / 3));
		for (std::size_t i = 0; i < primitive.indices.size(); ++i) {
			indexBuffer[i] = primitive.indices[i];
		}

		auto vertexBuffer = static_cast<Vec3A*>(rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX,
			0, RTC_FORMAT_FLOAT3, sizeof(Vec3A), primitive.positions.size()));
		
		for (std::size_t i = 0; i < primitive.positions.size(); ++i) {
			// Bake the transform into the positions
			auto worldPos = glm::vec3(primitive.transform * glm::vec4(primitive.positions[i], 1.0f));

			vertexBuffer[i].x = worldPos.x;
			vertexBuffer[i].y = worldPos.y;
			vertexBuffer[i].z = worldPos.z;
		}

		auto normalBuffer = static_cast<Vec3A*>(rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
			0, RTC_FORMAT_FLOAT3, sizeof(Vec3A), primitive.normals.size()));
		auto normalMatrix = glm::mat3(glm::transpose(glm::inverse(primitive.transform)));
		for (std::size_t i = 0; i < primitive.normals.size(); ++i) {
			// Bake the transform into the normals
			auto worldNormal = normalMatrix * primitive.normals[i];

			normalBuffer[i].x = worldNormal.x;
			normalBuffer[i].y = worldNormal.y;
			normalBuffer[i].z = worldNormal.z;
		}

		if (!primitive.tangents.empty()) {
			auto tangentBuffer = static_cast<Vec3A*>(rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
				1, RTC_FORMAT_FLOAT4, sizeof(Vec3A), primitive.tangents.size()));

			for (std::size_t i = 0; i < primitive.tangents.size(); ++i) {
				// Bake the transform into the tangents
				auto worldTangent = glm::vec4(normalMatrix * (glm::vec3(primitive.tangents[i]) * primitive.tangents[i].w), 1.0f);

				tangentBuffer[i].x = worldTangent.x;
				tangentBuffer[i].y = worldTangent.y;
				tangentBuffer[i].z = worldTangent.z;
				tangentBuffer[i].w = worldTangent.w;
			}
		}

		if (!primitive.texCoords.empty()) {
			auto texCoordBuffer = static_cast<Vec3A*>(rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
				2, RTC_FORMAT_FLOAT2, sizeof(Vec3A), primitive.texCoords.size()));

			for (std::size_t i = 0; i < primitive.texCoords.size(); ++i) {
				texCoordBuffer[i].x = primitive.texCoords[i].x;
				texCoordBuffer[i].y = primitive.texCoords[i].y;
			}
		}

		rtcCommitGeometry(mesh);
		auto geomID = rtcAttachGeometry(scene, mesh);
		rtcReleaseGeometry(mesh);

		Material material;
		material.albedoMap = primitive.albedoMap;
		material.normalMap = primitive.normalMap;
		material.roughnessMap = primitive.roughnessMap;
		material.baseColor = primitive.baseColor;
		material.roughness = primitive.roughness;
		material.metallic = primitive.metallic;
		materials.insert({ geomID, material });
	}

	rtcCommitScene(scene);
}

glm::vec3 PathTracer::trace(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& weight, int depth) const {
	if (depth > maxPathDepth) {
		return glm::vec3(0.0f);
	}

	RTCRayHit rayhit = { Ray(origin, dir, 0.001f, std::numeric_limits<float>::infinity()), Hit() };
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(scene, &context, &rayhit);

	if (rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
		return glm::vec3(0.0f);
	}

	glm::vec3 surfacePoint;
	surfacePoint.x = rayhit.ray.org_x + rayhit.ray.dir_x * rayhit.ray.tfar;
	surfacePoint.y = rayhit.ray.org_y + rayhit.ray.dir_y * rayhit.ray.tfar;
	surfacePoint.z = rayhit.ray.org_z + rayhit.ray.dir_z * rayhit.ray.tfar;

	alignas(16) glm::vec3 normal;
	rtcInterpolate0(rtcGetGeometry(scene, rayhit.hit.geomID), rayhit.hit.primID,
		rayhit.hit.u, rayhit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &normal[0], 3);
	normal = glm::normalize(normal);

	const Material& material = materials.at(rayhit.hit.geomID);
	glm::vec3 albedo;
	float roughness = material.roughness;
	if (material.albedoMap) {
		alignas(16) glm::vec2 texCoord;
		rtcInterpolate0(rtcGetGeometry(scene, rayhit.hit.geomID), rayhit.hit.primID,
			rayhit.hit.u, rayhit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 2, &texCoord[0], 2);

		albedo = gammaToLinear(glm::vec3(material.albedoMap->sample(texCoord))) * gammaToLinear(material.baseColor);
		roughness *= material.roughnessMap->sample(texCoord).x;

		/*
		alignas(16) glm::vec4 tangent;
		rtcInterpolate0(rtcGetGeometry(scene, rayhit.hit.geomID), rayhit.hit.primID,
			rayhit.hit.u, rayhit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, &tangent[0], 4);

		
		glm::vec3 normalMapN = glm::vec3(material.normalMap->sample({ texCoord[0], texCoord[1] }));
		normalMapN.x = normalMapN.x * 2.0f - 1.0f;
		normalMapN.y = normalMapN.y * 2.0f - 1.0f;

		glm::vec3 N = glm::normalize(normal);
		glm::vec3 T = glm::normalize(glm::vec3(tangent)) * tangent.w;
		glm::vec3 B = glm::normalize(glm::cross(N, glm::vec3(T)));
		glm::mat3 TBN = glm::mat3(T, B, N);
		normal = glm::normalize(TBN * normalMapN);
		*/
	}
	else {
		albedo = gammaToLinear(material.baseColor);
	}

	Ray occluderRay(surfacePoint + normal * 0.001f, glm::normalize(-light->direction), 0.0f, std::numeric_limits<float>::infinity());
	RTCIntersectContext occluderContext;
	rtcInitIntersectContext(&occluderContext);
	rtcOccluded1(scene, &occluderContext, &occluderRay);

	glm::vec3 diffuse = albedo * (1 - material.metallic);
	glm::vec3 specular = glm::mix(glm::vec3(0.04f), albedo, material.metallic);

	glm::vec3 directIllumination(0.0f);
	if (occluderRay.tfar >= 0.0f) {
		glm::vec3 L = glm::normalize(-light->direction);
		glm::vec3 V = glm::normalize(glm::vec3(-rayhit.ray.dir_x, -rayhit.ray.dir_y, -rayhit.ray.dir_z));

		glm::vec3 shadingDiffuse = brdfLambert(diffuse);
		glm::vec3 shadingSpecular = brdfCookTorrenceGGX(normal, V, L, std::max(0.01f, roughness), specular);
		glm::vec3 shading = shadingDiffuse + shadingSpecular;

		directIllumination = weight * shading * std::max(glm::dot(normal, L), 0.0f) * gammaToLinear(light->color) * light->power;
	}

	glm::vec3 indirectIllumination(0.0f);
	float rho = std::max(weight.x, std::max(weight.y, weight.z));
	if (uniformDist(randEngine) <= rho) {
		float diffLum = glm::dot(diffuse, glm::vec3(0.2126f, 0.7152f, 0.0722f));
		float specLum = glm::dot(specular, glm::vec3(0.2126f, 0.7152f, 0.0722f));
		float Pd = diffLum / (diffLum + specLum);
		float Ps = specLum / (diffLum + specLum);

		if (uniformDist(randEngine) <= Pd) {
			glm::vec3 brdf = brdfLambert(diffuse);
			glm::vec3 wi = sampleCosineHemisphere(normal);
			float pdf = pdfCosineHemisphere(normal, wi);

			float dotNL = glm::dot(normal, wi);
			assert(dotNL >= 0.0f);

			glm::vec3 newWeight = weight * dotNL * brdf / (pdf * rho * Pd);
			indirectIllumination = newWeight * trace(surfacePoint, wi, newWeight, depth + 1);
		}
		else {
			glm::vec3 V = glm::normalize(glm::vec3(-rayhit.ray.dir_x, -rayhit.ray.dir_y, -rayhit.ray.dir_z));
			glm::vec3 R = glm::normalize(glm::reflect(-V, normal));
			glm::vec3 wi = sampleGGX(R, glm::max(0.01f, roughness));
			float pdf = pdfGGX(R, wi, glm::max(0.01f, roughness));

			glm::vec3 brdf = brdfCookTorrenceGGX(normal, V, wi, glm::max(0.01f, roughness), specular);

			float dotNL = glm::dot(normal, wi);
			assert(dotNL >= 0.0f);

			glm::vec3 newWeight = weight * dotNL * brdf / (pdf * rho * Ps);
			indirectIllumination = newWeight * trace(surfacePoint, wi, newWeight, depth + 1);
		}
	}
	else {
		// Absorb
	}

	glm::vec3 illumination = directIllumination + indirectIllumination;
	if (depth >= clampDepth) {
		illumination = glm::clamp(illumination, 0.0f, clampRadiance);
	}

	return illumination;
}

void PathTracer::setLight(const DirectionalLight& light) {
	this->light = &light;
}

void PathTracer::setMaxPathDepth(unsigned int depth) {
	this->maxPathDepth = static_cast<int>(depth);
}

void PathTracer::setClampDepth(unsigned int depth) {
	this->clampDepth = static_cast<int>(depth);
}

void PathTracer::setClampRadiance(float radiance) {
	this->clampRadiance = radiance;
}