#include "TextRenderer.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

TextRenderer::TextRenderer(GLuint width, GLuint height)
    : screenWidth(width), screenHeight(height), textShader("assets/shaders/text.vs", "assets/shaders/text.fs")// 初始化文本着色器
{
    // 设置正交投影矩阵
    UpdateProjection(width, height);

    // 创建顶点数组和缓冲对象
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

TextRenderer::~TextRenderer() {
    // 清理OpenGL资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // 清理字符纹理
    for (auto& ch : Characters) {
        glDeleteTextures(1, &ch.second.TextureID);
    }
}

bool TextRenderer::LoadFont(const std::string& fontPath, GLuint fontSize) {
    Characters.clear();

    // 初始化FreeType库
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "[TextRenderer] ERROR: Could not init FreeType Library" << std::endl;
        return false;
    }

    // 加载字体文件
    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "[TextRenderer] ERROR: Failed to load font at " << fontPath << std::endl;
        FT_Done_FreeType(ft);
        return false;
    }

    // 设置字体大小
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // 禁用字节对齐限制（FreeType加载的位图不是4字节对齐的）
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 加载前128个ASCII字符
    for (GLubyte c = 0; c < 128; c++) {
        // 加载字符字形
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "[TextRenderer] ERROR: Failed to load Glyph for character '" << c << "'" << std::endl;
            continue;
        }

        // 生成纹理
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // 设置纹理选项
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 存储字符信息
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };

        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    // 清理FreeType资源
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // 恢复默认的4字节对齐
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    std::cout << "[TextRenderer] Loaded font: " << fontPath << " (size: " << fontSize << ")" << std::endl;
    return true;
}

void TextRenderer::RenderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
    // 启用混合（用于透明纹理）
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 激活着色器
    textShader.use();
    textShader.setVec3("textColor", color);

    // 激活纹理单元0
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // 遍历字符串中的所有字符
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        // 计算字符的位置
        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        // 计算字符的宽高
        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // 为每个字符更新VBO
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },  // 左下
            { xpos,     ypos,       0.0, 1.0 },  // 左上
            { xpos + w, ypos,       1.0, 1.0 },  // 右上

            { xpos,     ypos + h,   0.0, 0.0 },  // 左下
            { xpos + w, ypos,       1.0, 1.0 },  // 右上
            { xpos + w, ypos + h,   1.0, 0.0 }   // 右下
        };

        // 在四边形上绘制字符纹理
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // 更新VBO内存
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // 绘制四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 更新x位置，为下一个字符做准备（注意单位是1/64像素）
        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void TextRenderer::RenderTextWithAlpha(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec4 color) {
    // 启用混合（用于透明纹理）
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    if (!blendEnabled) {
        glEnable(GL_BLEND);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 激活着色器
    textShader.use();
    textShader.setVec4("textColor", color);

    // 激活纹理单元0
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // 遍历字符串中的所有字符
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        // 计算字符的位置
        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        // 计算字符的宽高
        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // 为每个字符更新VBO
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },  // 左下
            { xpos,     ypos,       0.0, 1.0 },  // 左上
            { xpos + w, ypos,       1.0, 1.0 },  // 右上

            { xpos,     ypos + h,   0.0, 0.0 },  // 左下
            { xpos + w, ypos,       1.0, 1.0 },  // 右上
            { xpos + w, ypos + h,   1.0, 0.0 }   // 右下
        };

        // 在四边形上绘制字符纹理
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // 更新VBO内存
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // 绘制四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 更新x位置，为下一个字符做准备
        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 恢复混合状态
    if (!blendEnabled) {
        glDisable(GL_BLEND);
    }
}

void TextRenderer::UpdateProjection(GLuint width, GLuint height) {
    screenWidth = width;
    screenHeight = height;

    // 创建正交投影矩阵（用于2D文本渲染）
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(width),
        static_cast<GLfloat>(height), 0.0f);

    textShader.use();
    textShader.setMat4("projection", projection);
}