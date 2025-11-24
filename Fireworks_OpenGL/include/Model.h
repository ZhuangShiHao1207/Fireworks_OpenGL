#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

class Model
{
public:
    // Model data
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    // Constructor
    Model(std::string const& path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // Draw the model
    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }
    
    // Check if model has textures
    bool HasTextures() const
    {
        return !textures_loaded.empty();
    }

private:
    // Load a model with supported ASSIMP extensions
    void loadModel(std::string const& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }

        directory = path.substr(0, path.find_last_of('/'));

        // Debug info
        std::cout << "  Model has " << scene->mNumMeshes << " meshes" << std::endl;
        std::cout << "  Model has " << scene->mNumMaterials << " materials" << std::endl;

        processNode(scene->mRootNode, scene);

        std::cout << "  Total textures loaded: " << textures_loaded.size() << std::endl;
    }

    // Process a node in a recursive fashion
    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // Debug: Print mesh info
        std::cout << "    Processing mesh: " << mesh->mName.C_Str() 
                  << " (" << mesh->mNumVertices << " vertices, " 
                  << mesh->mNumFaces << " faces)" << std::endl;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;

            // Positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // Normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            else
            {
                // If no normals, set a default up normal
                vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                std::cout << "      [WARNING] Mesh has no normals!" << std::endl;
            }

            // Texture Coordinates
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                // Tangent
                if (mesh->HasTangentsAndBitangents())
                {
                    vector.x = mesh->mTangents[i].x;
                    vector.y = mesh->mTangents[i].y;
                    vector.z = mesh->mTangents[i].z;
                    vertex.Tangent = vector;

                    // Bitangent
                    vector.x = mesh->mBitangents[i].x;
                    vector.y = mesh->mBitangents[i].y;
                    vector.z = mesh->mBitangents[i].z;
                    vertex.Bitangent = vector;
                }
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        // Indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // Diffuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // Specular maps
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // Normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // Height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            
            std::cout << "      Texture path from material: " << str.C_Str() << std::endl;

            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                Texture texture;
                
                // Check if it's an embedded texture (path starts with '*')
                if (str.C_Str()[0] == '*') {
                    std::cout << "      [INFO] Embedded texture detected: " << str.C_Str() << std::endl;
                    // For now, create a default texture
                    // TODO: Load embedded texture from aiScene
                    texture.id = TextureFromFile("", this->directory);
                } else {
                    texture.id = TextureFromFile(str.C_Str(), this->directory);
                }
                
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
};

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
    std::string filename = std::string(path);
    
    // Extract just the filename without path
    size_t lastSlash = filename.find_last_of("/\\");
    std::string bareFilename = (lastSlash != std::string::npos) ? filename.substr(lastSlash + 1) : filename;
    
    // Build the parent directory (去掉 source 子文件夹)
    std::string parentDir = directory;
    size_t sourcePos = parentDir.find("source");
    if (sourcePos != std::string::npos) {
        // 如果 directory 包含 "source"，向上一级
        parentDir = parentDir.substr(0, sourcePos);
        if (!parentDir.empty() && (parentDir.back() == '/' || parentDir.back() == '\\')) {
            parentDir.pop_back();
        }
    }
    
    // Try multiple possible paths for textures
    std::vector<std::string> possiblePaths = {
        // First try in same directory as model (for GLTF files)
        directory + "/" + bareFilename,
        directory + "\\" + bareFilename,
        
        // Then try in textures subfolder of parent directory
        parentDir + "/textures/" + bareFilename,
        parentDir + "\\textures\\" + bareFilename,
        
        // Try with original path relative to model directory
        directory + "/" + filename,
        directory + "\\" + filename,
        
        // Try in parent directory
        parentDir + "/" + bareFilename,
        parentDir + "\\" + bareFilename,
        
        // Try direct/absolute path
        filename
    };

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = nullptr;
    std::string loadedPath;
    
    // Try each possible path
    for (const auto& tryPath : possiblePaths) {
        data = stbi_load(tryPath.c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            loadedPath = tryPath;
            break;
        }
    }
    
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Improved texture filtering for sharper textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        // Use trilinear filtering for better quality
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Set mipmap bias for sharper textures (negative = sharper, positive = softer)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.5f);

        std::cout << "    ? Texture: " << bareFilename << " (" << width << "x" << height << ", " << nrComponents << " ch)" << std::endl;
        
        stbi_image_free(data);
    }
    else
    {
        std::cout << "    ? Failed: " << bareFilename << std::endl;
        
        // Create a default white texture
        unsigned char whitePixel[4] = { 255, 255, 255, 255 };
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        
        stbi_image_free(data);
    }

    return textureID;
}

#endif
