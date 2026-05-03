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

    unsigned int flags = aiProcess_JoinIdenticalVertices;

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

        AttributeStream posStream(ElementType::Float3);

        posStream.data.reserve(RHI::GetVertexElementSize(ElementType::Float3) * aim->mNumVertices);

        for (u32 i = 0; i < aim->mNumVertices; ++i) {
            auto vert = aim->mVertices[i];

            Append(posStream.data, float3(vert.x, vert.y, vert.z));
        }

        mesh.attributes.insert({Semantic::Position, std::move(posStream)});

        if (aim->HasNormals()) {
            AttributeStream normalStream(ElementType::Float3);

            normalStream.data.reserve(RHI::GetVertexElementSize(ElementType::Float3) * aim->mNumVertices);
            for (u32 i = 0; i < aim->mNumVertices; ++i) {
                auto norm = aim->mNormals[i];
                Append(normalStream.data, float3(norm.x, norm.y, norm.z));
            }

            mesh.attributes.insert({Semantic::Normal, std::move(normalStream)});
        }

        if (aim->HasTextureCoords(0)) {
            AttributeStream texCoordStream(ElementType::Float3);

            texCoordStream.data.reserve(RHI::GetVertexElementSize(ElementType::Float3) * aim->mNumVertices);
            for (u32 i = 0; i < aim->mNumVertices; ++i) {
                auto norm = aim->mTextureCoords[0][i];
                Append(texCoordStream.data, float3(norm.x, norm.y, norm.z));
            }

            mesh.attributes.insert({Semantic::TexCoord0, std::move(texCoordStream)});
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

// // ─────────────────────────────────────────────────────────────────────────────
// //  loadFirstMesh()
// //
// //  Convenience wrapper — merges ALL sub-meshes into a single Mesh,
// //  adjusting indices so they stay contiguous.
// // ─────────────────────────────────────────────────────────────────────────────
// inline MeshAsset loadFirstMesh(const std::string& path, const LoadOptions& opts = {})
// {
//     auto meshes = loadMeshes(path, opts);
//     if (meshes.empty())
//         throw std::runtime_error("No meshes found in: " + path);

//     if (meshes.size() == 1)
//         return std::move(meshes[0]);

//     // merge
//     Mesh merged;
//     for (auto& m : meshes) {
//         auto base = static_cast<unsigned int>(merged.vertices.size());
//         merged.vertices.insert(merged.vertices.end(), m.vertices.begin(), m.vertices.end());
//         for (unsigned int idx : m.indices)
//             merged.indices.push_back(idx + base);
//     }
//     return merged;
// }