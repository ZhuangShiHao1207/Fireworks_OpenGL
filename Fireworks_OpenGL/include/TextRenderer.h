#pragma once

#include <string>
#include <map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

// 字符结构，存储字符纹理和度量信息
struct Character {
    GLuint TextureID;       // 字形纹理ID
    glm::ivec2 Size;        // 字形大小
    glm::ivec2 Bearing;     // 从基线到字形左部/顶部的偏移值
    GLuint Advance;         // 距下一个字形原点的距离
};

class TextRenderer {
public:
    TextRenderer(GLuint width, GLuint height);
    ~TextRenderer();

    // 加载字体
    bool LoadFont(const std::string& fontPath, GLuint fontSize = 24);

    // 渲染文本
    void RenderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale,
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));

    // 更新投影矩阵（窗口大小改变时调用）
    void UpdateProjection(GLuint width, GLuint height);

    //在TextRenderer类中支持透明度
    void RenderTextWithAlpha(const std::string& text, GLfloat x, GLfloat y,
        GLfloat scale, glm::vec4 color);

private:
    std::map<GLchar, Character> Characters;  // 字符集
    Shader textShader;                       // 文本着色器
    GLuint VAO, VBO;                         // 顶点数组和缓冲对象
    GLuint screenWidth, screenHeight;        // 屏幕尺寸
};