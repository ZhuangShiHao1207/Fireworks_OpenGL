#pragma once
#include <glad/glad.h>
#include "Shader.h"

class PostProcessor {
public:
    PostProcessor(unsigned int width, unsigned int height);
    ~PostProcessor();

    void Bind();
    void Unbind();
    void Render();
    void SetFadeAlpha(float alpha);

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
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorBuffers[2];
    
    Shader* postShader;
    Shader* bloomShader;
    Shader* blurShader;
    
    int lastBlurTextureIndex;
};
