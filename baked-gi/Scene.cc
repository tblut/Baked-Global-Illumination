#include "Scene.hh"
#include "tinygltf/tiny_gltf.h"

#include <glow/fwd.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/data/TextureData.hh>
#include <glow/data/SurfaceData.hh>

namespace {
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
			return glow::ElementArrayBuffer::create(accessor.count, dataPtr.u8Ptr);
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			return glow::ElementArrayBuffer::create(accessor.count, dataPtr.u16Ptr);
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			return glow::ElementArrayBuffer::create(accessor.count, dataPtr.u32Ptr);
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

		if (image.component == 3) {
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

	Material createMaterial(const tinygltf::Material& material, const tinygltf::Model& model) {
		Material resultMat;
		
		auto it = material.values.find("baseColorFactor");
		if (it != material.values.end()) {
			resultMat.baseColor.r = static_cast<float>(it->second.ColorFactor()[0]);
			resultMat.baseColor.g = static_cast<float>(it->second.ColorFactor()[1]);
			resultMat.baseColor.b = static_cast<float>(it->second.ColorFactor()[2]);
		}

		it = material.values.find("roughnessFactor");
		if (it != material.values.end()) {
			resultMat.roughness = static_cast<float>(it->second.Factor());
		}

		it = material.values.find("metallicFactor");
		if (it != material.values.end()) {
			resultMat.metallic = static_cast<float>(it->second.Factor());
		}

		it = material.values.find("baseColorTexture");
		if (it != material.values.end()) {
			resultMat.colorMap = createTexture(model.textures[it->second.TextureIndex()], model, glow::ColorSpace::sRGB);
		}

		it = material.additionalValues.find("normalTexture");
		if (it != material.additionalValues.end()) {
			resultMat.normalMap = createTexture(model.textures[it->second.TextureIndex()], model, glow::ColorSpace::Linear);
		}

		return resultMat;
	}
}

void Scene::loadFromGltf(const std::string& path) {
	tinygltf::TinyGLTF context;
	tinygltf::Model model;
	std::string error;
	context.LoadASCIIFromFile(&model, &error, path);

	const auto& scene = model.scenes[model.defaultScene];
	for (int nodeIndex : scene.nodes) {
		const auto& node = model.nodes[nodeIndex];

		if (node.name == "Sun") {

		}
		else if (node.mesh != -1) {
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
					abs.push_back(createABForAccessor<glm::vec3>("aTangent", model.accessors[it->second], model));
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

				if (primitive.material == -1) {
					glow::error() << path << " contains a primitive without a material. Not supported!";
				}
				
				Material material = createMaterial(model.materials[primitive.material], model);
				meshes.push_back({ va, material });
			}
		}
	}
}

void Scene::render(const glow::camera::CameraBase& camera, RenderPipeline& pipeline) const {
	pipeline.render(camera, meshes);
}