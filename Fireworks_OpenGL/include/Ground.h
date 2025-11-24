#ifndef GROUND_H
#define GROUND_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <iostream>

/**
 * Ground 类
 * 用于创建和渲染带有雾效的无限地面
 * 支持纹理加载和 Blinn-Phong 光照
 */
class Ground {
public:
    unsigned int VAO, VBO, EBO;
    unsigned int textureID;
    glm::vec3 position;
    float size;
    bool hasTexture;
    float textureRepeat; // 纹理重复次数

    Ground(float groundSize = 50.0f, glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f)) 
        : size(groundSize), position(pos), hasTexture(false), textureID(0), textureRepeat(20.0f) {
        setupGround();
    }

    // 加载地面纹理
    void LoadTexture(const std::string& path, float repeat = 20.0f) {
        textureRepeat = repeat;
        
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
            
            // 使用新的纹理坐标更新地面
            updateTextureCoordinates();
            
            std::cout << "加载地面纹理: " << path << " (重复: " << repeat << "x)" << std::endl;
        }
        else {
            std::cout << "地面纹理加载失败: " << path << std::endl;
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
        
        // 地面顶点：位置、法线、纹理坐标
        float vertices[] = {
            // 位置                           // 法线              // 纹理坐标
            -halfSize, 0.0f,  halfSize,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f,  // 左上
             halfSize, 0.0f,  halfSize,    0.0f, 1.0f, 0.0f,    textureRepeat, 0.0f,  // 右上
             halfSize, 0.0f, -halfSize,    0.0f, 1.0f, 0.0f,    textureRepeat, textureRepeat, // 右下
            -halfSize, 0.0f, -halfSize,    0.0f, 1.0f, 0.0f,    0.0f, textureRepeat  // 左下
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

        // 位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // 法线属性
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // 纹理坐标属性
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void updateTextureCoordinates() {
        float halfSize = size / 2.0f;
        
        float vertices[] = {
            // 位置                           // 法线              // 纹理坐标
            -halfSize, 0.0f,  halfSize,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
             halfSize, 0.0f,  halfSize,    0.0f, 1.0f, 0.0f,    textureRepeat, 0.0f,
             halfSize, 0.0f, -halfSize,    0.0f, 1.0f, 0.0f,    textureRepeat, textureRepeat,
            -halfSize, 0.0f, -halfSize,    0.0f, 1.0f, 0.0f,    0.0f, textureRepeat
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    }
};

#endif
