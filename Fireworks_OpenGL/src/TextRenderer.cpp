#include "TextRenderer.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <codecvt>
#include <vector>
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

    // 加载常用中文字符（使用UTF-8编码）
    // 这里列出5句艺术字中会用到的所有中文字符
    std::vector<unsigned int> chineseChars = {
        // 第一句: 烟花粒子系统展示
        0x70DF, 0x82B1, 0x7C92, 0x5B50, 0x7CFB, 0x7EDF, 0x5C55, 0x793A,

        // 第二句: 1、展示天空盒、地面与模型
        0x0031, 0x3001, 0x5C55, 0x793A, 0x5929, 0x7A7A, 0x76D2, 0x3001, 0x5730, 0x9762, 0x4E0E, 0x6A21, 0x578B,

        // 第三句: 2、单独展示部分烟花
        0x0032, 0x3001, 0x5355, 0x72EC, 0x5C55, 0x793A, 0x90E8, 0x5206, 0x70DF, 0x82B1,

        // 第四句: 3、随机发射烟花
        0x0033, 0x3001, 0x968F, 0x673A, 0x53D1, 0x5C04, 0x70DF, 0x82B1,

        // 第五句: 4、点击地面发射烟花
        0x0034, 0x3001, 0x70B9, 0x51FB, 0x5730, 0x9762, 0x53D1, 0x5C04, 0x70DF, 0x82B1,

        // 第六句: 感谢观看
        0x611F, 0x8C22, 0x89C2, 0x770B,

        // 常用标点符号（复用已有标点，补充对齐）
        0x3001, 0x3002, 0xFF01, 0xFF1F, 0xFF0E
    };

    // 加载这些中文字符
    for (unsigned int charCode : chineseChars) {
        if (Characters.find(charCode) != Characters.end()) {
            continue; // 避免重复加载
        }

        if (FT_Load_Char(face, charCode, FT_LOAD_RENDER)) {
            // 如果字体不包含这个字符，跳过
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };

        Characters[charCode] = character;
    }

    // 清理FreeType资源
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // 恢复默认的4字节对齐
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    std::cout << "[TextRenderer] Loaded font: " << fontPath << " (size: " << fontSize << ")" << std::endl;
    return true;
}

void TextRenderer::RenderText(const std::string& text, GLfloat x, GLfloat y,
    GLfloat scale, glm::vec3 color) {
    RenderTextWithAlpha(text, x, y, scale, glm::vec4(color, 1.0f));
}

void TextRenderer::RenderTextWithAlpha(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec4 color) {
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    if (!blendEnabled) {
        glEnable(GL_BLEND);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    textShader.use();
    textShader.setVec4("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // UTF-8解码和渲染
    const char* c = text.c_str();
    while (*c) {
        unsigned int charCode = 0;
        
        // UTF-8解码
        if ((*c & 0x80) == 0) {
            // 1字节字符 (ASCII)
            charCode = *c;
            c += 1;
        } else if ((*c & 0xE0) == 0xC0) {
            // 2字节字符
            charCode = ((c[0] & 0x1F) << 6) | (c[1] & 0x3F);
            c += 2;
        } else if ((*c & 0xF0) == 0xE0) {
            // 3字节字符 (包括大部分中文)
            charCode = ((c[0] & 0x0F) << 12) | ((c[1] & 0x3F) << 6) | (c[2] & 0x3F);
            c += 3;
        } else if ((*c & 0xF8) == 0xF0) {
            // 4字节字符
            charCode = ((c[0] & 0x07) << 18) | ((c[1] & 0x3F) << 12) | ((c[2] & 0x3F) << 6) | (c[3] & 0x3F);
            c += 4;
        } else {
            // 无效UTF-8，跳过
            c++;
            continue;
        }

        auto it = Characters.find(charCode);
        if (it == Characters.end()) {
            // 如果字符未加载，使用问号代替
            it = Characters.find('?');
            if (it == Characters.end()) {
                continue;
            }
        }

        const Character& ch = it->second;
        
        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - ch.Bearing.y * scale;
        
        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 1.0 },
            { xpos,     ypos,       0.0, 0.0 },
            { xpos + w, ypos,       1.0, 0.0 },
            
            { xpos,     ypos + h,   0.0, 1.0 },
            { xpos + w, ypos,       1.0, 0.0 },
            { xpos + w, ypos + h,   1.0, 1.0 }
        };
        
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
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

float TextRenderer::CalculateTextWidth(const std::string& text, GLfloat scale) {
    float width = 0.0f;
    const char* c = text.c_str();

    while (*c) {
        unsigned int charCode = 0;

        // UTF-8解码
        if ((*c & 0x80) == 0) {
            charCode = *c;
            c += 1;
        }
        else if ((*c & 0xE0) == 0xC0) {
            charCode = ((c[0] & 0x1F) << 6) | (c[1] & 0x3F);
            c += 2;
        }
        else if ((*c & 0xF0) == 0xE0) {
            charCode = ((c[0] & 0x0F) << 12) | ((c[1] & 0x3F) << 6) | (c[2] & 0x3F);
            c += 3;
        }
        else if ((*c & 0xF8) == 0xF0) {
            charCode = ((c[0] & 0x07) << 18) | ((c[1] & 0x3F) << 12) | ((c[2] & 0x3F) << 6) | (c[3] & 0x3F);
            c += 4;
        }
        else {
            c++;
            continue;
        }

        auto it = Characters.find(charCode);
        if (it != Characters.end()) {
            width += (it->second.Advance >> 6) * scale;
        }
        else {
            // 使用空格宽度作为估计
            auto spaceIt = Characters.find(' ');
            if (spaceIt != Characters.end()) {
                width += (spaceIt->second.Advance >> 6) * scale;
            }
        }
    }

    return width;
}
