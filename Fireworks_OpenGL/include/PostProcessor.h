#pragma once
#include <glad/glad.h>
#include "Shader.h"

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
    void initBloomBuffers();
    void applyBloom();

    unsigned int FBO;
    unsigned int RBO;
    unsigned int textureColorBuffer;

    unsigned int quadVAO, quadVBO;
    unsigned int width, height;

    // Ping-pong 模糊 FBO
    unsigned int pingpongFBO[2];
    // 两个颜色缓冲
    unsigned int pingpongColorBuffers[2];

    // 着色器指针（初始化为 nullptr）
    Shader* postShader = nullptr;
    Shader* bloomShader = nullptr;
    Shader* blurShader = nullptr;

    // 记录模糊后结果所在的 pingpong 缓冲索引（0 或 1）
    int lastBlurTextureIndex = -1;
};
