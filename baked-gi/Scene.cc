#include "Scene.hh"
#include "tinygltf/tiny_gltf.h"

#include <glow/fwd.hh>
#include <glow/common/str_utils.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/data/TextureData.hh>
#include <glow/data/SurfaceData.hh>

#include <unordered_map>

namespace {
	using TextureCache = std::unordered_map<int, glow::SharedTexture2D>;

	union EabDataPtr {
		const unsigned char* u8Ptr;
		const unsigned short* u16Ptr;
		const unsigned int* u32Ptr;
	};

	template <typename AttrType>
	glow::SharedArrayBuffer createABForAccessor(const std::string& attribute,
			tinygltf::Accessor& accessor, const tinygltf::Model& model) {
		const auto& bufferView = model.bufferViews[accessor.bufferView];
		const auto& buffer = model.buffers[bufferView.buffer];

		int stride = accessor.ByteStride(bufferView);
		const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
		
		auto ab = glow::ArrayBuffer::create();
		ab->defineAttribute<AttrType>(attribute);
		ab->bind().setData(stride * accessor.count, dataPtr);
		return ab;
	}

	glow::SharedElementArrayBuffer createEABForAccessor(
			const tinygltf::Accessor& accessor, const tinygltf::Model& model) {
		const auto& bufferView = model.bufferViews[accessor.bufferView];
		const auto& buffer = model.buffers[bufferView.buffer];

		EabDataPtr dataPtr;
		dataPtr.u8Ptr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

		switch (accessor.componentType) {
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			return glow::ElementArrayBuffer::create(static_cast<int>(accessor.count), dataPtr.u8Ptr);
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			return glow::ElementArrayBuffer::create(static_cast<int>(accessor.count), dataPtr.u16Ptr);
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			return glow::ElementArrayBuffer::create(static_cast<int>(accessor.count), dataPtr.u32Ptr);
		}

		return nullptr;
	}

	glow::SharedTexture2D createTexture(const tinygltf::Texture& texture,
			const tinygltf::Model& model, glow::ColorSpace colorSpace) {
		const auto& image = model.images[texture.source];

		auto surface = std::make_shared<glow::SurfaceData>();
		auto dataPtr = image.image.data();
		surface->setData(std::vector<char>(dataPtr, dataPtr + image.width * image.height * image.component));
		surface->setType(GL_UNSIGNED_BYTE);
		surface->setMipmapLevel(0);
		surface->setWidth(image.width);
		surface->setHeight(image.height);

		auto tex = std::make_shared<glow::TextureData>();
		tex->setWidth(image.width);
		tex->setHeight(image.height);
		tex->addSurface(surface);

		GLenum iformatRGB;
		GLenum iformatRGBA;

		switch (colorSpace)
		{
		case glow::ColorSpace::Linear:
			iformatRGB = GL_RGB;
			iformatRGBA = GL_RGBA;
			break;
		case glow::ColorSpace::sRGB:
		default:
			iformatRGB = GL_SRGB;
			iformatRGBA = GL_SRGB_ALPHA;
			break;
		}

		if (image.component == 1) {
			surface->setFormat(GL_RGB);
			tex->setPreferredInternalFormat(iformatRGB);
			
			// convert grey to rgb
			std::vector<char> newData;
			const auto& oldData = surface->getData();
			auto oldSize = oldData.size();
			newData.resize(oldData.size() * 3);
			for (std::size_t i = 0; i < oldSize; ++i)
			{
				auto d = oldData[i];
				newData[i * 3 + 0] = d;
				newData[i * 3 + 1] = d;
				newData[i * 3 + 2] = d;
			}
			surface->setData(newData);
		}
		else if (image.component == 3) {
			tex->setPreferredInternalFormat(iformatRGB);
			surface->setFormat(GL_RGB);
		}
		else if (image.component == 4) {
			tex->setPreferredInternalFormat(iformatRGBA);
			surface->setFormat(GL_RGBA);
		}
		else {
			glow::error() << "Unsupported image component count";
		}

		const auto& sampler = model.samplers[texture.sampler];
		tex->setAnisotropicFiltering(16.0f);
		tex->setMagFilter(sampler.magFilter);
		tex->setMinFilter(sampler.minFilter);
		tex->setWrapS(sampler.wrapS);
		tex->setWrapT(sampler.wrapT);

		return glow::Texture2D::createFromData(tex);
	}

	Material createMaterial(const tinygltf::Material& material, const tinygltf::Model& model, TextureCache& textureCache) {
		Material resultMat;
		
		{
			auto it = material.values.find("baseColorFactor");
			if (it != material.values.end()) {
				resultMat.baseColor.r = static_cast<float>(it->second.ColorFactor()[0]);
				resultMat.baseColor.g = static_cast<float>(it->second.ColorFactor()[1]);
				resultMat.baseColor.b = static_cast<float>(it->second.ColorFactor()[2]);
			}
		}

		{
			auto it = material.values.find("roughnessFactor");
			if (it != material.values.end()) {
				resultMat.roughness = static_cast<float>(it->second.Factor());
			}
		}

		{
			auto it = material.values.find("metallicFactor");
			if (it != material.values.end()) {
				resultMat.metallic = static_cast<float>(it->second.Factor());
			}
		}

		{
			auto it = material.values.find("baseColorTexture");
			if (it != material.values.end()) {
				auto textureIndex = it->second.TextureIndex();
				auto texIt = textureCache.find(textureIndex);

				if (texIt == textureCache.end()) {
					resultMat.colorMap = createTexture(model.textures[textureIndex], model, glow::ColorSpace::sRGB);
					textureCache.insert({ textureIndex, resultMat.colorMap });
				}
				else {
					resultMat.colorMap = texIt->second;
				}
			}
		}

		{
			auto it = material.values.find("metallicRoughnessTexture");
			if (it != material.values.end()) {
				auto textureIndex = it->second.TextureIndex();
				auto texIt = textureCache.find(textureIndex);

				if (texIt == textureCache.end()) {
					resultMat.roughnessMap = createTexture(model.textures[textureIndex], model, glow::ColorSpace::Linear);
					textureCache.insert({ textureIndex, resultMat.roughnessMap });
				}
				else {
					resultMat.roughnessMap = texIt->second;
				}
			}
		}

		{
			auto it = material.additionalValues.find("normalTexture");
			if (it != material.additionalValues.end()) {
				auto textureIndex = it->second.TextureIndex();
				auto texIt = textureCache.find(textureIndex);

				if (texIt == textureCache.end()) {
					resultMat.normalMap = createTexture(model.textures[textureIndex], model, glow::ColorSpace::Linear);
					textureCache.insert({ textureIndex, resultMat.normalMap });
				}
				else {
					resultMat.normalMap = texIt->second;
				}
			}
		}
		
		return resultMat;
	}

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
}

void Scene::loadFromGltf(const std::string& path) {
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

	TextureCache textureCache;
	const auto& scene = model.scenes[model.defaultScene];

	for (int nodeIndex : scene.nodes) {
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
				std::vector<glow::SharedArrayBuffer> abs;
				
				auto it = primitive.attributes.find("POSITION");
				if (it == primitive.attributes.end()) {
					glow::error() << path << " contains a primitive without positions. Not supported!";
				}
				abs.push_back(createABForAccessor<glm::vec3>("aPosition", model.accessors[it->second], model));
				
				it = primitive.attributes.find("NORMAL");
				if (it == primitive.attributes.end()) {
					glow::error() << path << " contains a primitive without normals. Not supported!";
				}
				abs.push_back(createABForAccessor<glm::vec3>("aNormal", model.accessors[it->second], model));

				it = primitive.attributes.find("TANGENT");
				if (it != primitive.attributes.end()) {
					abs.push_back(createABForAccessor<glm::vec4>("aTangent", model.accessors[it->second], model));
				}
				
				it = primitive.attributes.find("TEXCOORD_0");
				if (it != primitive.attributes.end()) {
					abs.push_back(createABForAccessor<glm::vec2>("aTexCoord", model.accessors[it->second], model));
				}

				if (primitive.indices == -1) {
					glow::error() << path << " contains a primitive without indices. Not supported!";
				}

				glow::SharedElementArrayBuffer eab = createEABForAccessor(model.accessors[primitive.indices], model);
				auto va = glow::VertexArray::create(abs, eab, primitive.mode);

				Material material;
				if (primitive.material == -1) {
					glow::warning() << path << " contains a primitive without a material!";
				}
				else {
					material = createMaterial(model.materials[primitive.material], model, textureCache);
				}

				meshes.push_back({ va, material, transform });
			}
		}
	}
}

void Scene::render(RenderPipeline& pipeline) const {
	pipeline.render(meshes);
}

DirectionalLight& Scene::getSun() {
	return sun;
}

const DirectionalLight& Scene::getSun() const {
	return sun;
}