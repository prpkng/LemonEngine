#pragma once

#include "Lemon/Core.h"
#include <stdexcept>
#include <string>
#include <vector>

#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include "MeshAsset.h"
#include "RHI/Helpers/Builders.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace Lemon;
using namespace Lemon::RHI;

// ─── Load flags (combine with |) ─────────────────────────────────────────────
struct LoadOptions {
    bool leftHanded      = true; // flip V for OpenGL
    bool generateNormals = true; // smooth normals if missing
    bool triangulate     = true; // force triangles
};

template <typename T> void Append(std::vector<u8>& dst, const T& value)
{
    size_t offset = dst.size();
    dst.resize(offset + sizeof(T));
    memcpy(dst.data() + offset, &value, sizeof(T));
}

inline std::vector<MeshAsset> loadMeshes(const std::string& path, const LoadOptions& opts = {})
{

    unsigned int flags = aiProcess_CalcTangentSpace | aiProcess_ImproveCacheLocality;

    if (opts.triangulate)
        flags |= aiProcess_Triangulate;
    if (opts.leftHanded)
        flags |= aiProcess_ConvertToLeftHanded;
    if (opts.generateNormals)
        flags |= aiProcess_GenSmoothNormals;

    Assimp::Importer importer;
    const aiScene*   scene = importer.ReadFile(path, flags);

    if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) {
        throw std::runtime_error(std::string("Assimp: ") + importer.GetErrorString());
    }

    std::vector<MeshAsset> result;
    result.reserve(scene->mNumMeshes);

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* aim = scene->mMeshes[m];
        MeshAsset     mesh;

        // ── vertices ──────────────────────────────────────────────────────────
        // mesh.attributes.insert({Semantic::Position, });

        mesh.positions.reserve(aim->mNumVertices);

        for (u32 i = 0; i < aim->mNumVertices; ++i) {
            auto vert = aim->mVertices[i];

            mesh.positions.emplace_back(vert.x, vert.y, vert.z);
        }

        if (aim->HasNormals()) {

            mesh.normals.reserve(aim->mNumVertices);
            for (u32 i = 0; i < aim->mNumVertices; ++i) {
                auto norm = aim->mNormals[i];
            
                mesh.normals.emplace_back(norm.x, norm.y, norm.z);
            }
        }

        if (aim->HasTextureCoords(0)) {

            mesh.uvs.reserve(aim->mNumVertices);
            for (u32 i = 0; i < aim->mNumVertices; ++i) {
                auto uv = aim->mTextureCoords[0][i];

                mesh.uvs.emplace_back(uv.x, uv.y);
            }
        }

        // --- Indices ---------------

        std::vector<u32> indices;

        for (u32 i = 0; i < aim->mNumFaces; ++i) {
            const auto& face = aim->mFaces[i];

            for (u32 j = 0; j < face.mNumIndices; ++j)
                indices.push_back(face.mIndices[j]);
        }

        mesh.indices = std::move(indices);
        result.push_back(std::move(mesh));
    }

    return result;
}