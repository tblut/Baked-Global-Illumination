#include "Importer.hh"

#include <fstream>

#include <glm/glm.hpp>

#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>

#include <glow/common/profiling.hh>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

static glm::vec3 aiCast(aiVector3D const& v)
{
    return {v.x, v.y, v.z};
}
static glm::vec4 aiCast(aiColor4D const& v)
{
    return {v.r, v.g, v.b, v.a};
}

glow::assimp::Importer::Importer() {}

glow::SharedVertexArray glow::assimp::Importer::load(const std::string& filename)
{
    GLOW_ACTION();

    using namespace assimp;

    if (!std::ifstream(filename).good())
    {
        error() << "Error loading `" << filename << "' with Assimp.";
        error() << "  File not found/not readable";
        return nullptr;
    }

    uint32_t flags = aiProcess_SortByPType;

    if (mTriangulate)
        flags |= aiProcess_Triangulate;
    if (mCalculateTangents)
        flags |= aiProcess_CalcTangentSpace;
    if (mGenerateSmoothNormal)
        flags |= aiProcess_GenSmoothNormals;
    if (mGenerateUVCoords)
        flags |= aiProcess_GenUVCoords;
    if (mPreTransformVertices)
        flags |= aiProcess_PreTransformVertices;

    Assimp::Importer importer;
    auto scene = importer.ReadFile(filename, flags);

    if (!scene)
    {
        error() << "Error loading `" << filename << "' with Assimp.";
        error() << "  " << importer.GetErrorString();
        return nullptr;
    }


    if (!scene->HasMeshes())
    {
        error() << "File `" << filename << "' has no meshes.";
        return nullptr;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<std::vector<glm::vec2>> texCoords;
    std::vector<std::vector<glm::vec4>> colors;
    std::vector<uint32_t> indices;

    auto baseIdx = 0u;
    for (auto i = 0u; i < scene->mNumMeshes; ++i)
    {
        auto const& mesh = scene->mMeshes[i];
        auto colorsCnt = mesh->GetNumColorChannels();
        auto texCoordsCnt = mesh->GetNumUVChannels();

        if (texCoords.empty())
            texCoords.resize(texCoordsCnt);
        else if (texCoords.size() != texCoordsCnt)
        {
            error() << "File `" << filename << "':";
            error() << "  contains inconsistent texture coordinate counts";
            return nullptr;
        }

        if (colors.empty())
            colors.resize(colorsCnt);
        else if (colors.size() != colorsCnt)
        {
            error() << "File `" << filename << "':";
            error() << "  contains inconsistent vertex color counts";
            return nullptr;
        }

        // add faces
        auto fCnt = mesh->mNumFaces;
        for (auto f = 0u; f < fCnt; ++f)
        {
            auto const& face = mesh->mFaces[f];
            if (face.mNumIndices != 3)
            {
                error() << "File `" << filename << "':.";
                error() << "  non-3 faces not implemented/supported";
                return nullptr;
            }

            for (auto fi = 0u; fi < face.mNumIndices; ++fi)
            {
                indices.push_back(baseIdx + face.mIndices[fi]);
            }
        }

        // add vertices
        auto vCnt = mesh->mNumVertices;
        for (auto v = 0u; v < vCnt; ++v)
        {
            positions.push_back(aiCast(mesh->mVertices[v]));

            if (mesh->HasNormals())
                normals.push_back(aiCast(mesh->mNormals[v]));
            if (mesh->HasTangentsAndBitangents())
                tangents.push_back(aiCast(mesh->mTangents[v]));

            for (auto t = 0u; t < texCoordsCnt; ++t)
                texCoords[t].push_back((glm::vec2)aiCast(mesh->mTextureCoords[t][v]));
            for (auto t = 0u; t < colorsCnt; ++t)
                colors[t].push_back(aiCast(mesh->mColors[t][v]));
        }

        baseIdx = positions.size();
    }

    std::vector<SharedArrayBuffer> abs;

    if (!positions.empty())
    {
        auto ab = ArrayBuffer::create();
        ab->defineAttribute<glm::vec3>("aPosition");
        ab->bind().setData(positions);
        abs.push_back(ab);
    }
    if (!normals.empty())
    {
        auto ab = ArrayBuffer::create();
        ab->defineAttribute<glm::vec3>("aNormal");
        ab->bind().setData(normals);
        abs.push_back(ab);
    }
    if (!tangents.empty())
    {
        auto ab = ArrayBuffer::create();
        ab->defineAttribute<glm::vec3>("aTangent");
        ab->bind().setData(tangents);
        abs.push_back(ab);
    }
    for (auto i = 0u; i < colors.size(); ++i)
    {
        auto ab = ArrayBuffer::create();
        if (i == 0)
            ab->defineAttribute<glm::vec4>("aColor");
        else
            ab->defineAttribute<glm::vec4>("aColor" + std::to_string(i + 1));
        ab->bind().setData(colors[i]);
        abs.push_back(ab);
    }
    for (auto i = 0u; i < texCoords.size(); ++i)
    {
        auto ab = ArrayBuffer::create();
        if (i == 0)
            ab->defineAttribute<glm::vec2>("aTexCoord");
        else
            ab->defineAttribute<glm::vec2>("aTexCoord" + std::to_string(i + 1));
        ab->bind().setData(texCoords[i]);
        abs.push_back(ab);
    }

    for (auto const& ab : abs)
        ab->setObjectLabel(ab->getAttributes()[0].name + " of " + filename);

    auto eab = ElementArrayBuffer::create(indices);
    eab->setObjectLabel(filename);
    auto va = VertexArray::create(abs, eab, GL_TRIANGLES);
    va->setObjectLabel(filename);
    return va;
}
