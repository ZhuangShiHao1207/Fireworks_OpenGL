#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "include/Shader.h"
#include "include/Camera.h"
#include "include/Skybox.h"
#include "include/Ground.h"
#include "include/Model.h"
#include "include/PointLight.h"
#include "include/FireworkParticleSystem.h"

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

// 光源管理器（全局变量，以便在 processInput 中访问）
PointLightManager lightManager;

// 烟花粒子系统（全局变量，以便在 processInput 中访问）
FireworkParticleSystem fireworkSystem;

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

    Shader skyboxShader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    Shader groundShader("assets/shaders/ground.vs", "assets/shaders/ground.fs");
    Shader modelShader("assets/shaders/model.vs", "assets/shaders/model.fs");

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

    // 添加场景永久光源
    // 光源围绕书本模型放置
    // 前左光源 - 柔和暖色
    lightManager.AddPermanentLight(
        glm::vec3(-3.0f, 3.0f, 3.0f),      // 位置：前左，上方
        glm::vec3(1.0f, 0.95f, 0.9f),      // 颜色：暖白色
        3.0f                                // 强度：温和
    );
    
    // 前右光源 - 冷白色
    lightManager.AddPermanentLight(
        glm::vec3(3.0f, 3.0f, 3.0f),       // 位置：前右，上方
        glm::vec3(0.9f, 0.95f, 1.0f),      // 颜色：冷白色/蓝色
        3.0f                                // 强度：温和
    );
    
    // 后中光源 - 中性白色
    lightManager.AddPermanentLight(
        glm::vec3(0.0f, 4.0f, -3.0f),      // 位置：后中，上方
        glm::vec3(1.0f, 1.0f, 1.0f),       // 颜色：纯白色
        3.0f                               // 强度：稍强
    );
    
    // 顶部光源 - 环境补光
    lightManager.AddPermanentLight(
        glm::vec3(0.0f, 5.0f, 0.0f),       // 位置：正上方
        glm::vec3(0.95f, 0.95f, 1.0f),     // 颜色：淡蓝色
        3.0f                                // 强度：微弱
    );

    std::cout << "\n=== 烟花 OpenGL ===" << std::endl;
    std::cout << "模型: 书本模型 (使用 Assimp 加载)" << std::endl;
    std::cout << "场景光源: 已添加 4 个永久光源" << std::endl;
    std::cout << "控制说明:" << std::endl;
    std::cout << "  WASD - 移动摄像机" << std::endl;
    std::cout << "  Space/Shift - 上升/下降" << std::endl;
    std::cout << "  鼠标 - 环顾四周（启用后）" << std::endl;
    std::cout << "  R - 切换轨道模式" << std::endl;
    std::cout << "  F - 聚焦中心" << std::endl;
    std::cout << "  M - 切换鼠标控制（重要：按 M 启用摄像机旋转）" << std::endl;
    std::cout << "  T - 测试：添加临时烟花光源（持续 5 秒）" << std::endl;
    std::cout << "  1-4 - 发射烟花（球形、环形、心形、星形）" << std::endl;
    std::cout << "  ESC - 退出" << std::endl;
    std::cout << "\n[信息] 鼠标默认为自由模式。按 M 锁定/解锁鼠标以控制摄像机。\n" << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        //不要把具体逻辑放在这里（如矩阵计算/设置大量参数属性），这里尽量调用函数
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
		//处理输入
        processInput(window);

        // 更新光源管理器（移除过期的临时光源）
        lightManager.Update(deltaTime);
        camera.UpdateOrbitMode(deltaTime);

        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // 从管理器获取光源数据
        int numLights = lightManager.GetLightCount();
        auto lightPositions = lightManager.GetPositions();
        auto lightColors = lightManager.GetColors();
        auto lightIntensities = lightManager.GetIntensities();

        // 绘制地面（带 Blinn-Phong 光照和雾效）
        groundShader.use();
        groundShader.setMat4("projection", projection);
        groundShader.setMat4("view", view);
        groundShader.setMat4("model", ground.GetModelMatrix());
        groundShader.setVec3("viewPos", camera.Position);
        groundShader.setVec3("groundColor", glm::vec3(0.2f, 0.25f, 0.3f));
        groundShader.setBool("useTexture", ground.hasTexture);
        
        // 地面材质属性
        groundShader.setFloat("groundShininess", 32.0f);        // 地面较低光泽度
        groundShader.setFloat("groundSpecularStrength", 0.3f);  // 水面适度镜面反射
        
        groundShader.setVec3("fogColor", glm::vec3(0.05f, 0.05f, 0.1f));
        groundShader.setFloat("fogDensity", 0.02f);
        groundShader.setFloat("fogStart", 30.0f);
        
        if (ground.hasTexture) {
            groundShader.setInt("groundTexture", 0);
        }
        
        // 设置地面光源
        groundShader.setInt("numLights", numLights);
        for (int i = 0; i < numLights && i < 16; i++)
        {
            std::string index = std::to_string(i);
            groundShader.setVec3("lightPositions[" + index + "]", lightPositions[i]);
            groundShader.setVec3("lightColors[" + index + "]", lightColors[i]);
            groundShader.setFloat("lightIntensities[" + index + "]", lightIntensities[i]);
        }
        
        ground.Draw();

        // 绘制书本模型
        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        
        // 将书本放置在地面上方（3.0 单位高度）
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 3.0f, 0.0f));
        
        // 旋转书本使其平放在地面上
        // 绕 X 轴旋转 -90 度使其水平
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        
        // 缩放书本
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 0.25f));
        
        modelShader.setMat4("model", modelMatrix);
        modelShader.setVec3("viewPos", camera.Position);
        
        // 检查模型是否实际加载了纹理
        modelShader.setBool("hasTexture", island.HasTextures());
        
        // 材质属性（用于镜面反射）
        modelShader.setFloat("materialShininess", 64.0f);   // 镜面高光锐度（32-128 典型值）
        modelShader.setFloat("specularStrength", 0.5f);     // 镜面高光强度（0.0-1.0）
        
        modelShader.setVec3("fogColor", glm::vec3(0.05f, 0.05f, 0.1f));
        modelShader.setFloat("fogDensity", 0.02f);
        modelShader.setFloat("fogStart", 30.0f);
        
        // 设置模型光源（与地面相同）
        modelShader.setInt("numLights", numLights);
        for (int i = 0; i < numLights && i < 16; i++)
        {
            std::string index = std::to_string(i);
            modelShader.setVec3("lightPositions[" + index + "]", lightPositions[i]);
            modelShader.setVec3("lightColors[" + index + "]", lightColors[i]);
            modelShader.setFloat("lightIntensities[" + index + "]", lightIntensities[i]);
        }
        
        island.Draw(modelShader);

        // 更新烟花粒子系统
        fireworkSystem.update(deltaTime);
        // 设置烟花粒子系统的视图和投影矩阵
        fireworkSystem.setViewProj(view, projection);
        
        // 渲染烟花粒子系统
        fireworkSystem.render();

        // 绘制天空盒（最后渲染以优化）
        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setInt("skybox", 0);
        
        skybox.Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
