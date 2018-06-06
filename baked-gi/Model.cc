#include "Model.hh"
#include "RenderPipeline.hh"

#include <fstream>

#include <glm/glm.hpp>

#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/data/TextureData.hh>
#include <glow/common/str_utils.hh>

#include <glow/common/profiling.hh>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

using namespace glow;

static glm::vec3 aiCast(aiVector3D const& v) {
	return { v.x, v.y, v.z };
}

static glm::vec3 aiCast(aiColor3D const& v) {
	return { v.r, v.g, v.b };
}

static glm::vec4 aiCast(aiColor4D const& v) {
	return { v.r, v.g, v.b, v.a };
}

static std::string getFilename(const std::string& path) {
	auto minPosA = path.find_last_of('/');
	if (minPosA == std::string::npos) {
		minPosA = 0;
	}

	auto minPosB = path.find_last_of('\\');
	if (minPosB == std::string::npos) {
		minPosB = 0;
	}

	auto minPos = minPosA < minPosB ? minPosB : minPosA;

	return path.substr(minPos == 0 ? 0 : minPos + 1);
}

void Model::loadFromFile(const std::string& path, const std::string& texturesPath) {
	GLOW_ACTION();

	if (!std::ifstream(path).good()) {
		error() << "Error loading `" << path << "' with Assimp.";
		error() << "  File not found/not readable";
		return;
	}

	uint32_t flags = aiProcess_SortByPType;
	flags |= aiProcess_Triangulate;
	flags |= aiProcess_CalcTangentSpace;
	flags |= aiProcess_GenSmoothNormals;
	flags |= aiProcess_GenUVCoords;
	flags |= aiProcess_PreTransformVertices;
	flags |= aiProcess_FlipUVs;

	Assimp::Importer importer;
	auto scene = importer.ReadFile(path, flags);

	if (!scene) {
		error() << "Error loading `" << path << "' with Assimp.";
		error() << "  " << importer.GetErrorString();
		return;
	}

	if (!scene->HasMeshes()) {
		error() << "File `" << path << "' has no meshes.";
		return;
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec2> texCoords;
		std::vector<uint32_t> indices;

		const auto& mesh = scene->mMeshes[i];

		for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
		{
			auto const& face = mesh->mFaces[f];
			for (auto fi = 0u; fi < face.mNumIndices; ++fi)
			{
				indices.push_back(face.mIndices[fi]);
			}
		}
		
		auto vCnt = mesh->mNumVertices;
		for (auto v = 0u; v < vCnt; ++v)
		{
			positions.push_back(aiCast(mesh->mVertices[v]));

			if (mesh->HasNormals())
				normals.push_back(aiCast(mesh->mNormals[v]));
			if (mesh->HasTangentsAndBitangents())
				tangents.push_back(aiCast(mesh->mTangents[v]));

			if (mesh->HasTextureCoords(0))
				texCoords.push_back((glm::vec2)aiCast(mesh->mTextureCoords[0][v]));
		}
		
		std::vector<SharedArrayBuffer> abs;

		if (!positions.empty()) {
			auto ab = ArrayBuffer::create();
			ab->defineAttribute<glm::vec3>("aPosition");
			ab->bind().setData(positions);
			abs.push_back(ab);
		}
		if (!normals.empty()) {
			auto ab = ArrayBuffer::create();
			ab->defineAttribute<glm::vec3>("aNormal");
			ab->bind().setData(normals);
			abs.push_back(ab);
		}
		if (!tangents.empty()) {
			auto ab = ArrayBuffer::create();
			ab->defineAttribute<glm::vec3>("aTangent");
			ab->bind().setData(tangents);
			abs.push_back(ab);
		}
		if (!texCoords.empty()) {
			auto ab = ArrayBuffer::create();
			ab->defineAttribute<glm::vec2>("aTexCoord");
			ab->bind().setData(texCoords);
			abs.push_back(ab);
		}
		
		for (auto const& ab : abs)
			ab->setObjectLabel(ab->getAttributes()[0].name + " of " + path);
		
		auto eab = ElementArrayBuffer::create(indices);
		eab->setObjectLabel(path);
		auto va = VertexArray::create(abs, eab, GL_TRIANGLES);
		va->setObjectLabel(path);
		
		const auto& material = scene->mMaterials[mesh->mMaterialIndex];

		SharedTexture2D diffuseTexture;
		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString tempPath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &tempPath);
			std::string path = texturesPath + getFilename(tempPath.C_Str());
			auto it = textures.find(path);
			if (it != textures.end()) {
				diffuseTexture = it->second;
			}
			else {
				diffuseTexture = Texture2D::createFromFile(path, ColorSpace::sRGB);
				textures.insert(std::make_pair(path, diffuseTexture));
			}
		}
		
		SharedTexture2D normalTexture;
		if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
			aiString tempPath;
			material->GetTexture(aiTextureType_NORMALS, 0, &tempPath);
			std::string path = texturesPath + getFilename(tempPath.C_Str());
			auto it = textures.find(path);
			if (it != textures.end()) {
				normalTexture = it->second;
			}
			else {
				normalTexture = Texture2D::createFromFile(path, ColorSpace::Linear);
				textures.insert(std::make_pair(path, normalTexture));
			}
		}

		if (diffuseTexture && !normalTexture) {
			if (!defaultNormalMap) {
				defaultNormalMap = Texture2D::createFromFile(glow::util::pathOf(__FILE__) + "/textures/blue.png", ColorSpace::Linear);
			}
			normalTexture = defaultNormalMap;
		}

		aiColor3D baseColor(1, 1, 1);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);

		float shininess = 1.0f;
		material->Get(AI_MATKEY_SHININESS, shininess);
		float roughness = std::max(0.0f, 1.0f - shininess / 1000.0f);

		meshes.push_back({ va, diffuseTexture, normalTexture, roughness, 0.0f, aiCast(baseColor) });
	}
}

void Model::render(const glow::camera::CameraBase& camera, RenderPipeline& pipeline) const {
	pipeline.render(camera, meshes);
}