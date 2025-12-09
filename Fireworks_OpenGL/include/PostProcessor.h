#pragma once
#include <glad/glad.h>
#include "include/Shader.h"

class PostProcessor {
public:
    PostProcessor(unsigned int width, unsigned int height);
    ~PostProcessor();

    // 在渲染场景前调用（绑定 FBO）
    void Bind();
    // 在渲染场景后调用（解绑 FBO，恢复默认帧缓冲）
    void Unbind();
    // 绘制屏幕四边形并应用后处理着色器
    void Render();

    unsigned int GetTexture() const { return textureColorBuffer; }

private:
    void initFramebuffer();
    void initRenderData();

    unsigned int FBO;
    unsigned int RBO;
    unsigned int textureColorBuffer;

    unsigned int quadVAO, quadVBO;
    unsigned int width, height;

    Shader postShader;
};
