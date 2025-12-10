#include "PostProcessor.h"
#include <iostream>

static const char* defaultVertexSrc = "assets/shaders/post.vs";
static const char* defaultFragmentSrc = "assets/shaders/post.fs";
static const char* bloomVertexSrc = "assets/shaders/bloom.vs";
static const char* bloomFragmentSrc = "assets/shaders/bloom.fs";
static const char* blurVertexSrc = "assets/shaders/blur.vs";
static const char* blurFragmentSrc = "assets/shaders/blur.fs";

PostProcessor::PostProcessor(unsigned int w, unsigned int h)
: FBO(0), RBO(0), textureColorBuffer(0), quadVAO(0), quadVBO(0),
width(w), height(h)
//postShader(defaultVertexSrc, defaultFragmentSrc), 
//bloomShader(bloomVertexSrc, bloomFragmentSrc), 
//blurShader(blurVertexSrc, blurFragmentSrc)
{
	 std::cout << "PostProcessor initialized" << std::endl;
	 if (!glIsEnabled(GL_BLEND)) {
		 std::cerr << "OpenGL context not ready!" << std::endl;
	 }

	 pingpongFBO[0] = pingpongFBO[1] = 0;
	 pingpongColorBuffers[0] = pingpongColorBuffers[1] = 0;

	 // 不new应当也没有问题
	 postShader  = new Shader(defaultVertexSrc, defaultFragmentSrc);
	 bloomShader = new Shader(bloomVertexSrc, bloomFragmentSrc);
	 blurShader  = new Shader(blurVertexSrc, blurFragmentSrc);

	 // 初始化帧缓冲、渲染数据、Bloom缓冲
	 try {
		 initFramebuffer();
		 initRenderData();
		 initBloomBuffers();
	 } catch (const std::exception& e) {
		 std::cerr << "[PostProcessor] Exception during initialization: " << e.what() << std::endl;
	 }
}

PostProcessor::~PostProcessor()
{
	std::cout << "~PostProcessor --> quadVAO = " << quadVAO << ", quadVBO = " << quadVBO
		<< ", texttureColorBuffer = " << textureColorBuffer << ", RBO = " << RBO << ", FBO = " << FBO << std::endl;
	if (quadVBO) {
		glDeleteBuffers(1, &quadVBO);
		std::cout << "Delete VBO" << std::endl;
	 }
	if (quadVAO) {
		glDeleteVertexArrays(1, &quadVAO);
		std::cout << "Delete VAO" << std::endl;
	 }
	if (textureColorBuffer) {
		glDeleteTextures(1, &textureColorBuffer);
		std::cout << "Delete textureColorBuffer" << std::endl;
	 }
	if (RBO) {
		glDeleteRenderbuffers(1, &RBO);
		std::cout << "Delete RBO" << std::endl;
	 }
	if (FBO) {
		glDeleteFramebuffers(1, &FBO);
		std::cout << "Delete FBO" << std::endl;
	 }

	 for (int i = 0; i < 2; ++i) {
		 if (pingpongFBO[i]) glDeleteFramebuffers(1, &pingpongFBO[i]);
		 if (pingpongColorBuffers[i]) glDeleteTextures(1, &pingpongColorBuffers[i]);
	 }

	 std::cout << "Delete pingpong and pingpongColorBuffer" << std::endl;

	 delete bloomShader;
	 delete blurShader;
	 delete postShader;
}

void PostProcessor::initFramebuffer()
{
	 // 创建帧缓冲
	 glGenFramebuffers(1, &FBO);
	 glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	 // 创建颜色纹理附件
	 glGenTextures(1, &textureColorBuffer);
	 glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	 // std::cout << "textureColorbuffer: " << textureColorBuffer << std::endl;

	 // Use HDR (float) render target so bright values are preserved for bloom
	 glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA16F, width, height,0, GL_RGBA, GL_FLOAT, NULL);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	 // clamp to edge so sampling near borders during blur doesn't introduce artifacts
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	 glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer,0);

	 // Tell OpenGL we will draw into color attachment0 for this FBO
	 GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	 glDrawBuffers(1, drawBuffers);

	 // 创建渲染缓冲对象（深度 + 模板）
	 glGenRenderbuffers(1, &RBO);
	 glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	 glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	 glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	 // 检查FBO完整性
	 if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	 std::cerr << "[PostProcessor] Framebuffer not complete!" << std::endl;

	 //重新绑定默认帧缓冲
	 glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void PostProcessor::initRenderData()
{
	 postShader->use();
	 postShader->setBool("useBloom", true);
	 postShader->setFloat("exposure", 1.0f);
	 // 定义屏幕四边形顶点数据
	 float quadVertices[] = {
	 // positions // texCoords
	 -1.0f,1.0f,0.0f,1.0f,
	 -1.0f, -1.0f,0.0f,0.0f,
	1.0f, -1.0f,1.0f,0.0f,

	 -1.0f,1.0f,0.0f,1.0f,
	1.0f, -1.0f,1.0f,0.0f,
	1.0f,1.0f,1.0f,1.0f
	 };
	 glGenVertexArrays(1, &quadVAO);
	 // std::cout << "Generated VAO ID: " << quadVAO << std::endl;
	 glGenBuffers(1, &quadVBO);
	 // std::cout << "Generated VBO ID: " << quadVBO << std::endl;
	
	 // 只有bind之后才能识别到对象？
	 //if (!glIsVertexArray(quadVAO) || !glIsBuffer(quadVBO)) {
		// std::cerr << "[PostProcessor] Error: Failed to generate VAO/VBO!" << std::endl;
	 //}

	 if (quadVAO == 0 || quadVBO == 0) {
		 std::cerr << "[PostProcessor] Error: Failed to generate VAO/VBO!" << std::endl;
	 }

	 glBindVertexArray(quadVAO);
	 glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	 glEnableVertexAttribArray(0);
	 glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE,4 * sizeof(float), (void*)0);
	 glEnableVertexAttribArray(1);
	 glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE,4 * sizeof(float), (void*)(2 * sizeof(float)));
	 glBindVertexArray(0);
}

void PostProcessor::initBloomBuffers()
{
	// 创建两个 ping-pong 帧缓冲和颜色纹理
	 glGenFramebuffers(2, pingpongFBO);
	 glGenTextures(2, pingpongColorBuffers);

	 for (int i =0; i < 2; i++) {
		 // 初始化FBO + color attachment
		 glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		 glBindTexture(GL_TEXTURE_2D, pingpongColorBuffers[i]);
		 glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA16F, width, height,0, GL_RGBA, GL_FLOAT, NULL);
		 
		 // 图像过滤模式与 Wrap 模式 --> clamp to edge to prevent artifacts
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		 // 将纹理附着到 FBO
		 glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorBuffers[i],0);

		 // Ensure this FBO draws to color attachment0
		 GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
		 glDrawBuffers(1, drawBuffers);

		 if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		 std::cerr << "[PostProcessor] Pingpong framebuffer not complete!" << std::endl;
	 }

	 glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void PostProcessor::Bind()
{
	// std::cout << "Bind PostProcessor" << std::endl;
	if (!glIsFramebuffer(FBO)) std::cerr << "[PostProcessor::Bind] Warning: FBO not valid!" << std::endl;
	 glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	 glViewport(0,0, width, height);
}

void PostProcessor::Unbind()
{
	 // std::cout << "Unbind" << std::endl;
	 glBindFramebuffer(GL_FRAMEBUFFER,0);
	 // restore viewport to window size (assume same as width/height)
	 glViewport(0,0, width, height);
}

void PostProcessor::applyBloom()
{
	// std::cout << "ApplyBloom" << std::endl;
	// ------------------ Step 0: 检查 FBO/纹理 ------------------
	if (!glIsFramebuffer(pingpongFBO[0]) || !glIsFramebuffer(pingpongFBO[1])) {
		std::cerr << "[applyBloom] Pingpong FBO not properly initialized!" << std::endl;
		lastBlurTextureIndex = -1;
		return;
	}
	if (!glIsTexture(pingpongColorBuffers[0]) || !glIsTexture(pingpongColorBuffers[1])) {
		std::cerr << "[applyBloom] Pingpong textures not properly initialized!" << std::endl;
		lastBlurTextureIndex = -1;
		return;
	}
	if (!glIsTexture(textureColorBuffer)) {
		std::cerr << "[applyBloom] Scene texture not valid!" << std::endl;
		lastBlurTextureIndex = -1;
		return;
	}

	// ------------------ Step 1: 提取亮部 ------------------
	glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
	glViewport(0, 0, width, height);
	bloomShader->use();
	bloomShader->setInt("scene", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);

	if (!glIsVertexArray(quadVAO)) std::cerr << "[applyBloom] Warning: quadVAO not valid!" << std::endl;
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// ------------------ Step 2: 高斯模糊（ping-pong） ------------------
	bool horizontal = true;
	int blurPasses = 20;
	int readTex = 0; // 初始读取 pingpongColorBuffers[0]

	for (int i = 0; i < blurPasses; ++i) {
		int writeFBO = horizontal ? 1 : 0;

		if (!glIsFramebuffer(pingpongFBO[writeFBO]) || !glIsTexture(pingpongColorBuffers[readTex])) {
			std::cerr << "[applyBloom] Pingpong FBO/Texture invalid at pass " << i
				<< " (writeFBO=" << writeFBO
				<< ", readTex=" << readTex << ")" << std::endl;
			lastBlurTextureIndex = -1;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[writeFBO]);
		glViewport(0, 0, width, height);

		blurShader->use();
		blurShader->setInt("horizontal", horizontal);
		blurShader->setInt("image", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pingpongColorBuffers[readTex]);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		readTex = writeFBO;
		horizontal = !horizontal;
	}

	lastBlurTextureIndex = readTex;

	// 恢复默认帧缓冲
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 绑定默认帧缓冲后渲染四边形
void PostProcessor::Render()
{
	 // std::cout << "Render" << std::endl;
	 // Run bloom extraction + blur pass
	 applyBloom();

	 // 渲染合成：原场景 + 模糊后的辉光（假设 post shader 支持 bumble map 在纹理单元1）
	 // Disable depth test so fullscreen quad is always drawn
	 GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
	 glDisable(GL_DEPTH_TEST);

	 // 设置最终输出的片段着色器的uniform
	 postShader->use();
	 postShader->setInt("scene",0);
	 postShader->setInt("bloomBlur",1); // 读取辉光纹理

	 // 原场景纹理绑定到纹理单元0
	 glActiveTexture(GL_TEXTURE0);
	 if (!glIsTexture(textureColorBuffer)) std::cerr << "[Render] Warning: scene texture invalid!" << std::endl;
	 glBindTexture(GL_TEXTURE_2D, textureColorBuffer);

	 //绑定最终模糊结果到纹理单元1
	 glActiveTexture(GL_TEXTURE1);

	 if (lastBlurTextureIndex >= 0 && lastBlurTextureIndex < 2) {
		 if (!glIsTexture(pingpongColorBuffers[lastBlurTextureIndex])) {
			 std::cerr << "[Render] Warning: blur texture invalid!" << std::endl;
			 glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		 }
		 else {
			 glBindTexture(GL_TEXTURE_2D, pingpongColorBuffers[lastBlurTextureIndex]);
		 }
	 }
	 else {
		 glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	 }

	 // ensure viewport matches screen
	 glViewport(0,0, width, height);
	 if (!glIsVertexArray(quadVAO)) std::cerr << "[Render] Warning: quadVAO invalid!" << std::endl;

	 glBindVertexArray(quadVAO);
	 glDrawArrays(GL_TRIANGLES,0,6);

	 // restore depth test state
	 if (depthWasEnabled) glEnable(GL_DEPTH_TEST);
}