#include "StaticMeshletModel.h"

#include <iostream>


StaticMeshletModel::StaticMeshletModel(const char* modelPath, VkDescriptorSetLayout vkDescriptorSetLayout)
{


    VkResult vkResult = LoadModel_Static_PBR(modelPath, vkDescriptorSetLayout);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE,"StaticMeshletModel::StaticMeshletModel() : ""LoadModel_Static_PBR() failed (%d).\n", vkResult);
    }
}

StaticMeshletModel::~StaticMeshletModel()
{

    // PBR materials
    for (auto& matInfo : PBR_Materials)
    {
        if (matInfo.data)
        {
            delete matInfo.data;
            matInfo.data = nullptr;
        }
    }

    importer.FreeScene();

    scene = nullptr;
}

VkResult StaticMeshletModel::LoadModel_Static_PBR(
    const char* modelPath,
    VkDescriptorSetLayout vkDescriptorSetLayout)
{
    VkResult vkResult = VK_SUCCESS;

    // ============================================================
    // Load model
    // ============================================================

    scene = importer.ReadFile(
        modelPath,

        aiProcess_Triangulate
        | aiProcess_JoinIdenticalVertices
        | aiProcess_GenSmoothNormals
        | aiProcess_CalcTangentSpace
        | aiProcess_ImproveCacheLocality
        | aiProcess_SortByPType
        | aiProcess_FlipWindingOrder
    );

    if (!scene ||
        (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
    {
        std::cerr
            << "Assimp Error: "
            << importer.GetErrorString()
            << std::endl;

        fprintf(
            gpFILE,
            "StaticMeshletModel::LoadModel_Static_PBR() : "
            "Assimp Importer::ReadFile() failed.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // ============================================================
    // STATIC MODEL REQUIREMENT:
    // Currently this loader intentionally supports ONE mesh.
    // ============================================================

    if (scene->mNumMeshes != 1)
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::LoadModel_Static_PBR() : "
            "Expected exactly 1 mesh, found %u.\n",
            scene->mNumMeshes);

        return VK_ERROR_INITIALIZATION_FAILED;
    }

    aiMesh* mesh = scene->mMeshes[0];

    if (!mesh)
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::LoadModel_Static_PBR() : "
            "scene->mMeshes[0] is NULL.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // ============================================================
    // Materials
    // ============================================================

    uint16_t numMaterials =
        static_cast<uint16_t>(
            scene->mNumMaterials);

    for (unsigned int i = 0;
        i < numMaterials;
        ++i)
    {
        const aiMaterial* mat =
            scene->mMaterials[i];

        aiString matName;

        std::string materialName =
            "(unnamed)";

        if (AI_SUCCESS ==
            mat->Get(
                AI_MATKEY_NAME,
                matName))
        {
            materialName =
                matName.C_Str();
        }

        aiString tex;

        if (AI_SUCCESS ==
            mat->GetTexture(
                aiTextureType_DIFFUSE,
                0,
                &tex))
        {
            MaterialInfo info;

            std::string directory =
                std::filesystem::path(
                    tex.C_Str())
                .parent_path()
                .string();

            if (!directory.empty())
            {
                directory +=
                    std::filesystem::path::
                    preferred_separator;
            }

            info.materialName =
                materialName;

            info.path =
                directory;

            info.data =
                new Material_BasicPBR(
                    vkDescriptorSetLayout,
                    directory.c_str());

            PBR_Materials.push_back(
                info);
        }
    }

    // ============================================================
    // Vertex data
    // ============================================================

    vkVertices.clear();

    vkVertices.reserve(
        mesh->mNumVertices);

    // ============================================================
    // 32-bit indices
    // ============================================================


    {
        std::vector<uint32_t> vkIndices;

        vkIndices.reserve(
            mesh->mNumFaces * 3);

        // --------------------------------------------------------
        // Vertices
        // --------------------------------------------------------

        for (unsigned int i = 0;
            i < mesh->mNumVertices;
            ++i)
        {
            VertexData_Meshlet v{};

            // ----------------------------------------------------
            // Position
            // ----------------------------------------------------

            if (mesh->HasPositions())
            {
                v.pos =
                    glm::vec4(
                        mesh->mVertices[i].x,
                        mesh->mVertices[i].y,
                        mesh->mVertices[i].z,
                        1.0f);
            }

            // ----------------------------------------------------
            // Normal
            // ----------------------------------------------------

            if (mesh->HasNormals())
            {
                v.normal =
                    glm::vec4(
                        mesh->mNormals[i].x,
                        mesh->mNormals[i].y,
                        mesh->mNormals[i].z,
                        0.0f);
            }
            else
            {
                v.normal =
                    glm::vec4(
                        0.0f,
                        0.0f,
                        1.0f,
                        0.0f);
            }

            // ----------------------------------------------------
            // UV
            // ----------------------------------------------------

            if (mesh->HasTextureCoords(0))
            {
                v.texCoord =
                    glm::vec4(
                        mesh->mTextureCoords[0][i].x,
                        mesh->mTextureCoords[0][i].y,
                        0.0f,
                        0.0f);
            }
            else
            {
                v.texCoord =
                    glm::vec4(0.0f);
            }

            // ----------------------------------------------------
            // Tangent
            // ----------------------------------------------------

            if (mesh->HasTangentsAndBitangents())
            {
                glm::vec3 tangent(
                    mesh->mTangents[i].x,
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z);

                glm::vec3 bitangent(
                    mesh->mBitangents[i].x,
                    mesh->mBitangents[i].y,
                    mesh->mBitangents[i].z);

                glm::vec3 normal =
                    glm::normalize(v.normal);

                tangent =
                    glm::normalize(tangent);

                float handedness =
                    (glm::dot(
                        glm::cross(
                            tangent,
                            normal),
                        bitangent) < 0.0f)
                    ? -1.0f
                    : 1.0f;

                v.tangent =
                    glm::vec4(
                        tangent,
                        handedness);
            }
            else
            {
                v.tangent =
                    glm::vec4(
                        1.0f,
                        0.0f,
                        0.0f,
                        1.0f);
            }

            vkVertices.push_back(v);
        }

        // --------------------------------------------------------
        // Indices
        //
        // IMPORTANT:
        // Single mesh => Assimp's indices are already correct.
        // No vertex offset is required.
        // --------------------------------------------------------

        for (unsigned int f = 0;
            f < mesh->mNumFaces;
            ++f)
        {
            const aiFace& face =
                mesh->mFaces[f];

            for (unsigned int j = 0;
                j < face.mNumIndices;
                ++j)
            {
                vkIndices.push_back(
                    static_cast<uint32_t>(
                        face.mIndices[j]));
            }
        }

        
        vkResult = CreateMeshlets(vkIndices, vkVertices);

        if (vkResult != VK_SUCCESS)
        {
            return vkResult;
        }

    }



    // ============================================================
    // Done
    // ============================================================



    return VK_SUCCESS;
}


VkResult StaticMeshletModel::CreateMeshlets(
    const std::vector<uint32_t>& indices,
    const std::vector<VertexData_Meshlet>& vertices)
{
    // ============================================================
    // Meshlet limits
    // ============================================================

    constexpr size_t MAX_MESHLET_VERTICES = 64;
    constexpr size_t MAX_MESHLET_TRIANGLES = 126;


    if (indices.empty())
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::CreateMeshlets() : "
            "Index array is empty.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (vertices.empty())
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::CreateMeshlets() : "
            "Vertex array is empty.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }


    // ============================================================
    // Maximum number of meshlets required
    // ============================================================

    size_t maxMeshlets =
        meshopt_buildMeshletsBound(
            indices.size(),
            MAX_MESHLET_VERTICES,
            MAX_MESHLET_TRIANGLES);


    if (maxMeshlets == 0)
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::CreateMeshlets() : "
            "meshopt_buildMeshletsBound() returned 0.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }


    // ============================================================
    // Temporary meshlet storage
    // ============================================================

    std::vector<meshopt_Meshlet> tempMeshlets(
        maxMeshlets);

    std::vector<uint32_t> tempMeshletVertices(
        maxMeshlets *
        MAX_MESHLET_VERTICES);

    std::vector<unsigned char> tempMeshletTriangles(
        maxMeshlets *
        MAX_MESHLET_TRIANGLES *
        3);


    // ============================================================
    // Build meshlets
    // ============================================================

    size_t meshletCount =
        meshopt_buildMeshlets(
            tempMeshlets.data(),
            tempMeshletVertices.data(),
            tempMeshletTriangles.data(),

            indices.data(),
            indices.size(),

            &vertices[0].pos.x,
            vertices.size(),

            sizeof(VertexData_Meshlet),

            MAX_MESHLET_VERTICES,
            MAX_MESHLET_TRIANGLES,

            0.0f);


    if (meshletCount == 0)
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::CreateMeshlets() : "
            "meshopt_buildMeshlets() returned 0.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }


    fprintf(
        gpFILE,
        "StaticMeshletModel::CreateMeshlets() : "
        "Generated %zu meshlets.\n",
        meshletCount);


    tempMeshlets.resize(meshletCount);


    // ============================================================
    // Optimize each meshlet
    // ============================================================

    for (const meshopt_Meshlet& meshlet :
        tempMeshlets)
    {
        meshopt_optimizeMeshlet(
            tempMeshletVertices.data() +
            meshlet.vertex_offset,

            tempMeshletTriangles.data() +
            meshlet.triangle_offset,

            meshlet.triangle_count,
            meshlet.vertex_count);
    }


    // ============================================================
    // Determine actual used ranges
    // ============================================================

    size_t totalMeshletVertices = 0;
    size_t totalMeshletTriangles = 0;

    for (const meshopt_Meshlet& meshlet :
        tempMeshlets)
    {
        const size_t vertexEnd =
            static_cast<size_t>(
                meshlet.vertex_offset +
                meshlet.vertex_count);

        const size_t triangleEnd =
            static_cast<size_t>(
                meshlet.triangle_offset +
                meshlet.triangle_count * 3);

        totalMeshletVertices =
            std::max(
                totalMeshletVertices,
                vertexEnd);

        totalMeshletTriangles =
            std::max(
                totalMeshletTriangles,
                triangleEnd);
    }


    // ============================================================
    // Copy final meshlet data into class-owned arrays
    // ============================================================

    meshlets.clear();
    meshletVertices.clear();
    meshletTriangles.clear();


    meshlets.reserve(meshletCount);

    meshletVertices.reserve(
        totalMeshletVertices);

    meshletTriangles.reserve(
        totalMeshletTriangles);


    // ------------------------------------------------------------
    // Meshlet metadata
    // ------------------------------------------------------------

    for (const meshopt_Meshlet& src :
        tempMeshlets)
    {
        MeshletData dst{};

        // --------------------------------------------------------
        // Meshlet topology
        // --------------------------------------------------------

        dst.vertexOffset =
            static_cast<uint32_t>(
                src.vertex_offset);

        dst.triangleOffset =
            static_cast<uint32_t>(
                src.triangle_offset);

        dst.vertexCount =
            static_cast<uint32_t>(
                src.vertex_count);

        dst.triangleCount =
            static_cast<uint32_t>(
                src.triangle_count);


        // --------------------------------------------------------
        // Meshoptimizer meshlet bounds
        // --------------------------------------------------------

        meshopt_Bounds bounds =
            meshopt_computeMeshletBounds(
                tempMeshletVertices.data() +
                src.vertex_offset,

                tempMeshletTriangles.data() +
                src.triangle_offset,

                src.triangle_count,

                &vertices[0].pos.x,

                vertices.size(),

                sizeof(VertexData_Meshlet));


        // --------------------------------------------------------
        // Store bounding sphere
        //
        // xyz = center
        // w   = radius
        // --------------------------------------------------------

        dst.bounds =
            glm::vec4(
                bounds.center[0],
                bounds.center[1],
                bounds.center[2],
                bounds.radius);


        meshlets.push_back(dst);
    }


    // ------------------------------------------------------------
    // Meshlet vertex references
    // ------------------------------------------------------------

    meshletVertices.assign(
        tempMeshletVertices.begin(),
        tempMeshletVertices.begin() +
        totalMeshletVertices);


    // ------------------------------------------------------------
    // Meshlet triangle indices
    // ------------------------------------------------------------

    meshletTriangles.assign(
        tempMeshletTriangles.begin(),
        tempMeshletTriangles.begin() +
        totalMeshletTriangles);


    // ============================================================
    // Log generated data
    // ============================================================

    fprintf(
        gpFILE,
        "StaticMeshletModel::CreateMeshlets() : "
        "Vertices=%zu, Triangles=%zu.\n",
        meshletVertices.size(),
        meshletTriangles.size());


    // ============================================================
    // Create device-local SSBOs
    // ============================================================

    /*  
    VkResult vkResult =
        CreateMeshletSSBOs(vertices);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::CreateMeshlets() : "
            "CreateMeshletSSBOs() failed (%d).\n",
            vkResult);

        return vkResult;
    }


    fprintf(
        gpFILE,
        "StaticMeshletModel::CreateMeshlets() : "
        "Meshlet generation and GPU upload completed.\n");  
    */

    return VK_SUCCESS;
}


