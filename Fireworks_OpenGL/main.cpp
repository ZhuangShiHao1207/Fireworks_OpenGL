#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdexcept>

#include "include/Shader.h"
#include "include/Camera.h"
#include "include/Skybox.h"
#include "include/Ground.h"
#include "include/Model.h"
#include "include/PointLight.h"
#include "include/FireworkParticleSystem.h"
#include "include/PostProcessor.h"

// 输入处理相关
#include "include/InputHandler.h"

// 窗口设置
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 摄像机 - 调整为从上前方角度查看书本
Camera camera(glm::vec3(0.0f, 4.5f, 10.0f));  // 调整距离和高度以更好地查看书本
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 计时
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 鼠标控制
bool mouseEnabled = false;

// 场景灯光控制
bool sceneLightsEnabled = true;  // 默认开启，使用微弱环境光

// 光源管理器（全局变量，以便在 processInput 中访问）
// 应当不需要清理，不涉及到OpenGL的对象
PointLightManager lightManager;

// 烟花粒子系统（全局变量，以便在 processInput 中访问）
FireworkParticleSystem fireworkSystem;

// PostProcessor 全局指针（用于窗口缩放时更新）
PostProcessor* postProcessor = nullptr;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Firework System", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "创建 GLFW 窗口失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // 后续不能重复调用
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "初始化 GLAD 失败" << std::endl;
        return -1;
    }
    // 启用着色器控制点大小
    glEnable(GL_PROGRAM_POINT_SIZE);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 需要清理
    Shader *skyboxShader = new Shader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    Shader *groundShader = new Shader("assets/shaders/ground.vs", "assets/shaders/ground.fs");
    Shader *modelShader  = new Shader("assets/shaders/model.vs", "assets/shaders/model.fs");

    // 需要清理？
    Skybox skybox;
    Ground ground(100.0f);
    
    // 使用 Assimp 加载书本模型
    std::cout << "正在加载模型..." << std::endl;
    Model island("assets/model/book/source/TEST2.fbx");
    std::cout << "模型加载成功！" << std::endl;

    std::cout << "正在加载天空盒..." << std::endl;
    skybox.LoadCubemapSeparateFaces("assets/skybox/NightSkyHDRI007_4K");
    std::cout << "正在加载地面纹理..." << std::endl;
    ground.LoadTexture("assets/ground/sea.png");

    // 添加场景永久光源 - 微弱环境光，既能看到建筑色彩，又不影响烟花效果
    // 主光源 - 柔和白色，从上方照亮场景
    lightManager.AddPermanentLight(
        glm::vec3(0.0f, 8.0f, 5.0f),       // 位置：上方偏前
        glm::vec3(0.9f, 0.9f, 1.0f),       // 颜色：淡蓝白色
        1.5f                                // 强度：较弱（原来3.0）
    );
    
    // 辅助光源 - 补光，照亮书本侧面
    lightManager.AddPermanentLight(
        glm::vec3(-5.0f, 5.0f, 0.0f),      // 位置：左侧
        glm::vec3(1.0f, 0.95f, 0.9f),      // 颜色：暖白色
        1.2f                                // 强度：微弱
    );
    
    // 背景光源 - 填充阴影
    lightManager.AddPermanentLight(
        glm::vec3(5.0f, 5.0f, -5.0f),      // 位置：右后方
        glm::vec3(0.85f, 0.9f, 1.0f),      // 颜色：冷白色
        1.0f                                // 强度：很弱
    );
    
    // 地面补光 - 照亮地面
    lightManager.AddPermanentLight(
        glm::vec3(0.0f, 2.0f, 0.0f),       // 位置：较低位置
        glm::vec3(0.8f, 0.85f, 0.9f),      // 颜色：淡蓝色
        0.8f                                // 强度：极弱
    );

    std::cout << "\n=== Fireworks OpenGL ===" << std::endl;
    std::cout << "Model: Book model (loaded with Assimp)" << std::endl;
    std::cout << "Scene lights: 4 permanent lights added" << std::endl;
    std::cout << "Note: Fireworks create temporary lights that illuminate the scene" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Space/Shift - Up/Down" << std::endl;
    std::cout << "  Mouse - Look around (after enabled)" << std::endl;
    std::cout << "  R - Toggle orbit mode" << std::endl;
    std::cout << "  F - Focus center" << std::endl;
    std::cout << "  M - Toggle mouse control (Press M to enable camera rotation)" << std::endl;
    std::cout << "  L - Toggle scene lights (default ON - weak ambient)" << std::endl;
    std::cout << "  T - Test: Add temporary light (5s)" << std::endl;
    std::cout << "  1 - Launch Sphere firework (Red/Yellow)" << std::endl;
    std::cout << "  2 - Launch Ring firework (Cyan)" << std::endl;
    std::cout << "  3 - Launch Heart firework (Pink)" << std::endl;
    std::cout << "  4 - Launch MultiLayer firework (Blue)" << std::endl;
    std::cout << "  5 - Launch Spiral firework (Gold)" << std::endl;
    std::cout << "  6 - Launch Sphere firework (Purple) - All types have double explosion" << std::endl;
    std::cout << "  0 - Run auto test sequence" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "\n[Info] Mouse is free by default. Press M to lock/unlock mouse.\n" << std::endl;

    // 连接烟花系统与光源管理器
    fireworkSystem.setLightManager(&lightManager);

    // 测试模式标志
    bool autoTestMode = false;

    // std::cout << "Current Context: " << glfwGetCurrentContext() << std::endl;
    postProcessor = new PostProcessor(SCR_WIDTH, SCR_HEIGHT);

    while (!glfwWindowShouldClose(window))
    {
		// 所有的场景都将渲染到后处理对象的帧缓冲中
        postProcessor->Bind();

        //不要把具体逻辑放在这里（如矩阵计算/设置大量参数属性），这里尽量调用函数
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
		//处理输入
        processInput(window);

        // 检查是否按下0键启动测试
        static bool wasKey0Pressed = false;
        bool isKey0Pressed = (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS);
        if (isKey0Pressed && !wasKey0Pressed) {
            autoTestMode = !autoTestMode;
            std::cout << "[Test] Auto test mode: " << (autoTestMode ? "ON" : "OFF") << std::endl;
        }
        wasKey0Pressed = isKey0Pressed;

        // 如果测试模式开启，运行测试
        if (autoTestMode) {
            fireworkSystem.runTest(currentFrame);
        }

        // 更新光源管理器（移除过期的临时光源）
        lightManager.Update(deltaTime);
        camera.UpdateOrbitMode(deltaTime);

        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // 从管理器获取光源数据
        // 如果场景灯光关闭，只使用烟花产生的临时光源
        int numLights = lightManager.GetLightCount();
        auto lightPositions = lightManager.GetPositions();
        auto lightColors = lightManager.GetColors();
        auto lightIntensities = lightManager.GetIntensities();
        
        // 如果场景灯光关闭，将前4个永久光源的强度设为0
        if (!sceneLightsEnabled && numLights >= 4) {
            for (int i = 0; i < 4; i++) {
                lightIntensities[i] = 0.0f;
            }
        }

        // 绘制地面（带 Blinn-Phong 光照和雾效）
        groundShader->use();
        groundShader->setMat4("projection", projection);
        groundShader->setMat4("view", view);
        groundShader->setMat4("model", ground.GetModelMatrix());
        groundShader->setVec3("viewPos", camera.Position);
        groundShader->setVec3("groundColor", glm::vec3(0.2f, 0.25f, 0.3f));
        groundShader->setBool("useTexture", ground.hasTexture);
        
        // 地面材质属性
        groundShader->setFloat("groundShininess", 32.0f);        // 地面较低光泽度
        groundShader->setFloat("groundSpecularStrength", 0.3f);  // 水面适度镜面反射
        
        groundShader->setVec3("fogColor", glm::vec3(0.05f, 0.05f, 0.1f));
        groundShader->setFloat("fogDensity", 0.02f);
        groundShader->setFloat("fogStart", 30.0f);
        
        if (ground.hasTexture) {
            groundShader->setInt("groundTexture", 0);
        }
        
        // 设置地面光源
        groundShader->setInt("numLights", numLights);
        for (int i = 0; i < numLights && i < 16; i++)
        {
            std::string index = std::to_string(i);
            groundShader->setVec3("lightPositions[" + index + "]", lightPositions[i]);
            groundShader->setVec3("lightColors[" + index + "]", lightColors[i]);
            groundShader->setFloat("lightIntensities[" + index + "]", lightIntensities[i]);
        }
        
        ground.Draw();

        // 绘制书本模型
        modelShader->use();
        modelShader->setMat4("projection", projection);
        modelShader->setMat4("view", view);
        
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        
        // 将书本放置在地面上方（3.0 单位高度）
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 3.0f, 0.0f));
        
        // 旋转书本使其平放在地面上
        // 绕 X 轴旋转 -90 度使其水平
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        
        // 缩放书本
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 0.25f));
        
        modelShader->setMat4("model", modelMatrix);
        modelShader->setVec3("viewPos", camera.Position);
        
        // 检查模型是否实际加载了纹理
        modelShader->setBool("hasTexture", island.HasTextures());
        
        // 材质属性（用于镜面反射）
        modelShader->setFloat("materialShininess", 64.0f);   // 镜面高光锐度（32-128 典型值）
        modelShader->setFloat("specularStrength", 0.5f);     // 镜面高光强度（0.0-1.0）
        
        modelShader->setVec3("fogColor", glm::vec3(0.05f, 0.05f, 0.1f));
        modelShader->setFloat("fogDensity", 0.02f);
        modelShader->setFloat("fogStart", 30.0f);
        
        // 设置模型光源（与地面相同）
        modelShader->setInt("numLights", numLights);
        for (int i = 0; i < numLights && i < 16; i++)
        {
            std::string index = std::to_string(i);
            modelShader->setVec3("lightPositions[" + index + "]", lightPositions[i]);
            modelShader->setVec3("lightColors[" + index + "]", lightColors[i]);
            modelShader->setFloat("lightIntensities[" + index + "]", lightIntensities[i]);
        }
        
        // 传引用，不会发生拷贝
        island.Draw(*modelShader);

        // 绘制天空盒（先渲染，使用深度测试确保在最远处）
        glDepthFunc(GL_LEQUAL);
        skyboxShader->use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        skyboxShader->setMat4("view", skyboxView);
        skyboxShader->setMat4("projection", projection);
        skyboxShader->setInt("skybox", 0);
        glDepthFunc(GL_LESS);
        
		// 此时绘制完之后FBO中已经存在完整的场景内容
        skybox.Draw();

        // 更新烟花粒子系统
        fireworkSystem.update(deltaTime);
        // 设置烟花粒子系统的视图和投影矩阵
        fireworkSystem.setViewProj(view, projection);              

        // 渲染烟花粒子系统（在天空盒之后，会正确显示在前面）
        fireworkSystem.render();

		postProcessor->Unbind();
		postProcessor->Render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 必须在上下文被销毁前清理 OpenGL 资源
    try {
        // 先清理 PostProcessor（包含 Shader）
        if (postProcessor) {
            delete postProcessor;
            postProcessor = nullptr;
        }
        
        // 清理其他 Shader
        if (skyboxShader) {
            delete skyboxShader;
            skyboxShader = nullptr;
        }
        if (groundShader) {
            delete groundShader;
            groundShader = nullptr;
        }
        if (modelShader) {
            delete modelShader;
            modelShader = nullptr;
        }
        
        // 清理其他 OpenGL 资源
        fireworkSystem.cleanupGL();
        skybox.cleanup();
        ground.cleanup();
        
        std::cout << "OpenGL resources cleaned successfully" << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception during OpenGL cleanup: " << e.what() << std::endl;
    }

    // 最后销毁 GLFW（这会销毁 OpenGL 上下文）
    try {
        glfwTerminate();
        std::cout << "GLFW terminated successfully" << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception during GLFW termination: " << e.what() << std::endl;
    }
    return 0;
}
