#ifndef GROUND_H
#define GROUND_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <iostream>

class Ground {
public:
    unsigned int VAO, VBO, EBO;
    unsigned int textureID;
    glm::vec3 position;
    float size;
    bool hasTexture;

    Ground(float groundSize = 50.0f, glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f)) 
        : size(groundSize), position(pos), hasTexture(false), textureID(0) {
        setupGround();
    }

    // Load ground texture
    void LoadTexture(const std::string& path) {
        glGenTextures(1, &textureID);
        
        int width, height, nrChannels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 1) format = GL_RED;
            else if (nrChannels == 3) format = GL_RGB;
            else if (nrChannels == 4) format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
            hasTexture = true;
            std::cout << "Loaded ground texture: " << path << std::endl;
        }
        else {
            std::cout << "Failed to load ground texture: " << path << std::endl;
            stbi_image_free(data);
        }
    }

    void Draw() {
        if (hasTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
        }
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    glm::mat4 GetModelMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        return model;
    }

    ~Ground() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        if (hasTexture) {
            glDeleteTextures(1, &textureID);
        }
    }

private:
    void setupGround() {
        float halfSize = size / 2.0f;
        
        // Ground vertices with positions, normals, and texture coordinates
        float vertices[] = {
            // positions                           // normals              // texcoords
            -halfSize, 0.0f,  halfSize,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f,  // top-left
             halfSize, 0.0f,  halfSize,    0.0f, 1.0f, 0.0f,    10.0f, 0.0f,  // top-right
             halfSize, 0.0f, -halfSize,    0.0f, 1.0f, 0.0f,    10.0f, 10.0f, // bottom-right
            -halfSize, 0.0f, -halfSize,    0.0f, 1.0f, 0.0f,    0.0f, 10.0f  // bottom-left
        };

        unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }
};

#endif
