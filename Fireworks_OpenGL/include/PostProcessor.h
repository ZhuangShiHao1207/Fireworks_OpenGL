#pragma once
#include <glad/glad.h>
#include "Shader.h"

class PostProcessor {
public:
    PostProcessor(unsigned int width, unsigned int height);
    PostProcessor(const PostProcessor&) = delete;
    PostProcessor(PostProcessor&&) = delete;
    ~PostProcessor();

    // 在渲染场景前调用（绑定 FBO）
    void Bind();
    // 在渲染场景后调用（解绑 FBO，恢复默认帧缓冲）
    void Unbind();
    // 绘制屏幕四边形并应用后处理着色器
    void Render();
    // 设置渐变 alpha 值
    void SetFadeAlpha(float alpha);

    unsigned int GetTexture() const { return textureColorBuffer; }
    void SetBloomEnabled(bool enabled) {
        if (postShader) {
            postShader->use();
            postShader->setBool("useBloom", enabled);
        }
    }

    void SetExposure(float exp) {
        if (postShader) {
            postShader->use();
            postShader->setFloat("exposure", exp);
        }
    }

private:
    void initFramebuffer();
    void initRenderData();
    void initBloomBuffers();  // 初始化辉光用 FBO/纹理
    void applyBloom();        // 执行辉光后处理

    unsigned int FBO;
    unsigned int RBO;
    unsigned int textureColorBuffer;

    unsigned int quadVAO, quadVBO;
    unsigned int width, height;

    // Ping-pong 模糊 FBO
    unsigned int pingpongFBO[2];
    // 两个颜色纹理
    unsigned int pingpongColorBuffers[2];

    Shader *postShader = nullptr;
    Shader *bloomShader = nullptr; // 提取亮部
    Shader *blurShader = nullptr;  // 高斯模糊

    // 最终模糊结果所在的 pingpong纹理索引（0 或1）
    int lastBlurTextureIndex = -1; // -1 表示尚未生成模糊结果
};
