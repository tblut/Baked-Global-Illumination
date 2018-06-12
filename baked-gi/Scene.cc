#include "Scene.hh"
#include "PathTracer.hh"
#include "third-party/tiny_gltf.h"

#include <glow/fwd.hh>
#include <glow/common/str_utils.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/data/TextureData.hh>
#include <glow/data/SurfaceData.hh>

#include <unordered_map>
#include <algorithm>

namespace {
	glm::mat4 getTransformForNode(const tinygltf::Node& node) {
		glm::mat4 transform(1.0f);
		if (node.matrix.size() > 0) {
			const auto& m = node.matrix;
			transform = glm::mat4(
				static_cast<float>(m[0]), static_cast<float>(m[1]), static_cast<float>(m[2]), static_cast<float>(m[3]),
				static_cast<float>(m[4]), static_cast<float>(m[5]), static_cast<float>(m[6]), static_cast<float>(m[7]),
				static_cast<float>(m[8]), static_cast<float>(m[9]), static_cast<float>(m[10]), static_cast<float>(m[11]),
				static_cast<float>(m[12]), static_cast<float>(m[13]), static_cast<float>(m[14]), static_cast<float>(m[15]));
		}
		else {
			glm::vec3 translation(0.0f, 0.0f, 0.0f);
			if (node.translation.size() > 0) {
				translation.x = static_cast<float>(node.translation[0]);
				translation.y = static_cast<float>(node.translation[1]);
				translation.z = static_cast<float>(node.translation[2]);
			}

			glm::vec3 scale(1.0f, 1.0f, 1.0f);
			if (node.scale.size() > 0) {
				scale.x = static_cast<float>(node.scale[0]);
				scale.y = static_cast<float>(node.scale[1]);
				scale.z = static_cast<float>(node.scale[2]);
			}

			glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
			if (node.rotation.size() > 0) {
				rotation.x = static_cast<float>(node.rotation[0]);
				rotation.y = static_cast<float>(node.rotation[1]);
				rotation.z = static_cast<float>(node.rotation[2]);
				rotation.w = static_cast<float>(node.rotation[3]);
			}

			transform = glm::translate(translation) * glm::mat4_cast(rotation) * glm::scale(scale);
		}

		return transform;
	}

	template <typename DataType>
	std::vector<DataType> getDataFromAccessor(const tinygltf::Accessor& accessor, const tinygltf::Model& model) {
		const auto& bufferView = model.bufferViews[accessor.bufferView];
		const auto& buffer = model.buffers[bufferView.buffer];
		auto offset = bufferView.byteOffset + accessor.byteOffset;
		auto dataPtr = reinterpret_cast<const DataType*>(buffer.data.data() + offset);
		return std::vector<DataType>(dataPtr, dataPtr + accessor.count);
	}

	std::vector<unsigned int> getIndexDataFromAccessor(const tinygltf::Accessor& accessor, const tinygltf::Model& model) {
		std::vector<unsigned int> result;

		if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
			auto indices = getDataFromAccessor<unsigned char>(accessor, model);
			result.reserve(indices.size());
			std::copy(indices.begin(), indices.end(), std::back_inserter(result));
		}
		else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
			auto indices = getDataFromAccessor<unsigned short>(accessor, model);
			result.reserve(indices.size());
			std::copy(indices.begin(), indices.end(), std::back_inserter(result));
		}
		else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
			auto indices = getDataFromAccessor<unsigned int>(accessor, model);
			result.reserve(indices.size());
			std::copy(indices.begin(), indices.end(), std::back_inserter(result));
		}

		return result;
	}

	SharedImage getOrCreateTexture(int textureIndex, glow::ColorSpace colorSpace,
			const tinygltf::Model& model, std::vector<SharedImage>& images) {
		if (images[textureIndex]) {
			return images[textureIndex];
		}

		const auto& texture = model.textures[textureIndex];
		const auto& image = model.images[texture.source];
		const auto& sampler = model.samplers[texture.sampler];

		SharedImage result = std::make_shared<Image>(image.width, image.height, image.component, colorSpace);
		std::memcpy(result->getDataPtr(), image.image.data(), image.width * image.height * image.component);
		result->setWrapMode(sampler.wrapS, sampler.wrapT);
		images[textureIndex] = result;

		return result;
	}

	Primitive createPrimitive(const tinygltf::Primitive& primitive, tinygltf::Model& model, std::vector<SharedImage>& images) {
		Primitive p;
		p.mode = primitive.mode;

		auto it = primitive.attributes.find("POSITION");
		if (it == primitive.attributes.end()) {
			glow::error() << "The glTF model contains a primitive without positions. Not supported!";
		}
		p.positions = getDataFromAccessor<glm::vec3>(model.accessors[it->second], model);

		it = primitive.attributes.find("NORMAL");
		if (it == primitive.attributes.end()) {
			glow::error() << "The glTF model contains a primitive without normals. Not supported!";
		}
		p.normals = getDataFromAccessor<glm::vec3>(model.accessors[it->second], model);

		it = primitive.attributes.find("TANGENT");
		if (it != primitive.attributes.end()) {
			p.tangents = getDataFromAccessor<glm::vec4>(model.accessors[it->second], model);
		}

		it = primitive.attributes.find("TEXCOORD_0");
		if (it != primitive.attributes.end()) {
			auto texCoords = getDataFromAccessor<glm::vec2>(model.accessors[it->second], model);

			it = primitive.attributes.find("TEXCOORD_1");
			if (it != primitive.attributes.end()) {
				p.texCoords = std::move(texCoords);
				p.lightMapTexCoords = getDataFromAccessor<glm::vec2>(model.accessors[it->second], model);
			}
			else {
				p.lightMapTexCoords = std::move(texCoords);
			}
		}

		if (primitive.indices == -1) {
			glow::error() << "The glTF model contains a primitive without indices. Not supported!";
		}
		p.indices = getIndexDataFromAccessor(model.accessors[primitive.indices], model);

		if (primitive.material == -1) {
			glow::warning() << "The glTF model  contains a primitive without a material!";
		}
		else {
			const auto& material = model.materials[primitive.material];

			auto it = material.values.find("baseColorFactor");
			if (it != material.values.end()) {
				p.baseColor.r = static_cast<float>(it->second.ColorFactor()[0]);
				p.baseColor.g = static_cast<float>(it->second.ColorFactor()[1]);
				p.baseColor.b = static_cast<float>(it->second.ColorFactor()[2]);
			}

			it = material.values.find("roughnessFactor");
			if (it != material.values.end()) {
				p.roughness = static_cast<float>(it->second.Factor());
			}

			it = material.values.find("metallicFactor");
			if (it != material.values.end()) {
				p.metallic = static_cast<float>(it->second.Factor());
			}

			it = material.values.find("baseColorTexture");
			if (it != material.values.end()) {
				p.albedoMap = getOrCreateTexture(it->second.TextureIndex(), glow::ColorSpace::sRGB, model, images);
			}

			it = material.values.find("metallicRoughnessTexture");
			if (it != material.values.end()) {
				p.roughnessMap = getOrCreateTexture(it->second.TextureIndex(), glow::ColorSpace::Linear, model, images);
			}

			auto it2 = material.additionalValues.find("normalTexture");
			if (it2 != material.additionalValues.end()) {
				p.normalMap = getOrCreateTexture(it2->second.TextureIndex(), glow::ColorSpace::Linear, model, images);
			}
		}

		return p;
	}
}

void Scene::loadFromGltf(const std::string& path, bool makeRealtimeObjects) {
	tinygltf::TinyGLTF context;
	tinygltf::Model model;
	std::string error;

	std::string ending = glow::util::fileEndingOf(path);
	if (ending == ".gltf") {
		context.LoadASCIIFromFile(&model, &error, path);
	}
	else if (ending == ".glb") {
		context.LoadBinaryFromFile(&model, &error, path);
	}
	else {
		glow::error() << path << " is not a gltf file!";
		return;
	}

	// Allocate enough textures
	images.resize(model.textures.size());

	for (int nodeIndex : model.scenes[model.defaultScene].nodes) {
		const auto& node = model.nodes[nodeIndex];

		if (node.name == "Sun") {
			glm::mat4 transform = getTransformForNode(node);
			glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
			sun.direction = glm::mat3(transform) * direction;
		}
		else if (node.mesh != -1) {
			glm::mat4 transform = getTransformForNode(node);
			const auto& mesh = model.meshes[node.mesh];

			for (const auto& primitive : mesh.primitives) {
				Primitive p = createPrimitive(primitive, model, images);
				p.name = mesh.name;
				p.transform = transform;
				primitives.push_back(std::move(p));
			}
		}
	}

	// Create the realtime objects if necessary
	if (makeRealtimeObjects) {
		meshes.reserve(primitives.size());
		textures.resize(images.size());
		
		for (const auto& primitive : primitives) {
			Mesh mesh;
			mesh.transform = primitive.transform;

			// Geometry
			std::vector<glow::SharedArrayBuffer> abs;

			{
				auto ab = glow::ArrayBuffer::create();
				ab->defineAttribute<glm::vec3>("aPosition");
				ab->bind().setData(primitive.positions);
				abs.push_back(ab);
			}

			{
				auto ab = glow::ArrayBuffer::create();
				ab->defineAttribute<glm::vec3>("aNormal");
				ab->bind().setData(primitive.normals);
				abs.push_back(ab);
			}

			if (!primitive.tangents.empty()) {
				auto ab = glow::ArrayBuffer::create();
				ab->defineAttribute<glm::vec4>("aTangent");
				ab->bind().setData(primitive.tangents);
				abs.push_back(ab);
			}

			if (!primitive.texCoords.empty()) {
				auto ab = glow::ArrayBuffer::create();
				ab->defineAttribute<glm::vec2>("aTexCoord");
				ab->bind().setData(primitive.texCoords);
				abs.push_back(ab);
			}

			auto eab = glow::ElementArrayBuffer::create(primitive.indices);
			mesh.vao = glow::VertexArray::create(abs, eab, primitive.mode);

			// Material
			mesh.material.baseColor = primitive.baseColor;
			mesh.material.roughness = primitive.roughness;
			mesh.material.metallic = primitive.metallic;

			if (primitive.albedoMap) {
				auto pos = std::distance(images.begin(), std::find(images.begin(), images.end(), primitive.albedoMap));
				if (!textures[pos]) {
					textures[pos] = primitive.albedoMap->createTexture();
				}

				mesh.material.colorMap = textures[pos];
			}

			if (primitive.normalMap) {
				auto pos = std::distance(images.begin(), std::find(images.begin(), images.end(), primitive.normalMap));
				if (!textures[pos]) {
					textures[pos] = primitive.normalMap->createTexture();
				}

				mesh.material.normalMap = textures[pos];
			}

			if (primitive.roughnessMap) {
				auto pos = std::distance(images.begin(), std::find(images.begin(), images.end(), primitive.roughnessMap));
				if (!textures[pos]) {
					textures[pos] = primitive.roughnessMap->createTexture();
				}

				mesh.material.roughnessMap = textures[pos];
			}

			meshes.push_back(mesh);
		}
	}
}

void Scene::render(RenderPipeline& pipeline) const {
	pipeline.render(meshes);
}

void Scene::buildPathTracerScene(PathTracer& pathTracer) const {
	pathTracer.buildScene(primitives);
	pathTracer.setLight(sun);
}

DirectionalLight& Scene::getSun() {
	return sun;
}

const DirectionalLight& Scene::getSun() const {
	return sun;
}