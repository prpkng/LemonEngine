#pragma once

#include "Lemon/Core.h"
#include <stdexcept>
#include <string>
#include <vector>


#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <linalg.h>

using namespace linalg::aliases;

// ─── Vertex layout ────────────────────────────────────────────────────────────
struct Vertex {
    float3 position;
    float3 normal;
    float2 texcoord;
};

// ─── Per-mesh result ──────────────────────────────────────────────────────────
struct Mesh {
    std::vector<Vertex>       vertices;
    std::vector<u16> indices;
};

// ─── Load flags (combine with |) ─────────────────────────────────────────────
struct LoadOptions {
    bool leftHanded      = true; // flip V for OpenGL
    bool generateNormals = true; // smooth normals if missing
    bool triangulate     = true; // force triangles
};

// ─────────────────────────────────────────────────────────────────────────────
//  loadMeshes()
//
//  Returns one Mesh per sub-mesh in the file.
//  Throws std::runtime_error on failure.
// ─────────────────────────────────────────────────────────────────────────────
inline std::vector<Mesh> loadMeshes(const std::string& path, const LoadOptions& opts = {})
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

    std::vector<Mesh> result;
    result.reserve(scene->mNumMeshes);

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* aim = scene->mMeshes[m];
        Mesh          mesh;

        // ── vertices ──────────────────────────────────────────────────────────
        mesh.vertices.reserve(aim->mNumVertices);

        for (unsigned int i = 0; i < aim->mNumVertices; ++i) {
            Vertex v{};

            v.position[0] = aim->mVertices[i].x;
            v.position[1] = aim->mVertices[i].y;
            v.position[2] = aim->mVertices[i].z;

            if (aim->HasNormals()) {
                v.normal[0] = aim->mNormals[i].x;
                v.normal[1] = aim->mNormals[i].y;
                v.normal[2] = aim->mNormals[i].z;
            }

            // Assimp supports up to 8 UV channels; we take channel 0
            if (aim->HasTextureCoords(0)) {
                v.texcoord[0] = aim->mTextureCoords[0][i].x;
                v.texcoord[1] = aim->mTextureCoords[0][i].y;
            }

            mesh.vertices.push_back(v);
        }

        // ── indices ───────────────────────────────────────────────────────────
        mesh.indices.reserve(aim->mNumFaces * 3);

        for (unsigned int f = 0; f < aim->mNumFaces; ++f) {
            const aiFace& face = aim->mFaces[f];
            for (unsigned int idx = 0; idx < face.mNumIndices; ++idx)
                mesh.indices.push_back(face.mIndices[idx]);
        }

        result.push_back(std::move(mesh));
    }

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
//  loadFirstMesh()
//
//  Convenience wrapper — merges ALL sub-meshes into a single Mesh,
//  adjusting indices so they stay contiguous.
// ─────────────────────────────────────────────────────────────────────────────
inline Mesh loadFirstMesh(const std::string& path, const LoadOptions& opts = {})
{
    auto meshes = loadMeshes(path, opts);
    if (meshes.empty())
        throw std::runtime_error("No meshes found in: " + path);

    if (meshes.size() == 1)
        return std::move(meshes[0]);

    // merge
    Mesh merged;
    for (auto& m : meshes) {
        auto base = static_cast<unsigned int>(merged.vertices.size());
        merged.vertices.insert(merged.vertices.end(), m.vertices.begin(), m.vertices.end());
        for (unsigned int idx : m.indices)
            merged.indices.push_back(idx + base);
    }
    return merged;
}