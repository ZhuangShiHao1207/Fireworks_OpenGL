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

// 函数声明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

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

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "烟花粒子系统", NULL, NULL);
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
    std::cout << "  ESC - 退出" << std::endl;
    std::cout << "\n[信息] 鼠标默认为自由模式。按 M 锁定/解锁鼠标以控制摄像机。\n" << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

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

// 处理所有输入
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 摄像机移动
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    // 切换轨道模式
    static bool rKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rKeyPressed)
    {
        camera.ToggleOrbitMode();
        rKeyPressed = true;
        std::cout << "轨道模式: " << (camera.OrbitMode ? "开启" : "关闭") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
        rKeyPressed = false;

    // 聚焦中心
    static bool fKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyPressed)
    {
        camera.FocusOn(glm::vec3(0.0f, 5.0f, 0.0f));
        fKeyPressed = true;
        std::cout << "摄像机聚焦到中心" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
        fKeyPressed = false;

    // 切换鼠标控制
    static bool mKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !mKeyPressed)
    {
        mouseEnabled = !mouseEnabled;
        glfwSetInputMode(window, GLFW_CURSOR, mouseEnabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true; // 重置首次鼠标以避免摄像机跳动
        mKeyPressed = true;
        std::cout << "鼠标控制: " << (mouseEnabled ? "锁定（摄像机控制已启用）" : "自由（可移出窗口）") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
        mKeyPressed = false;

    // 测试：添加临时烟花光源
    static bool tKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tKeyPressed)
    {
        // 在摄像机位置临时添加一个烟花光源
        lightManager.AddTemporaryLight(
            camera.Position,                   // 位置：摄像机所在位置
            glm::vec3(1.0f, 0.8f, 0.6f),      // 颜色：柔和暖色
            10.0f,                            // 强度
            5.0f                              // 生命周期：5 秒
        );
        tKeyPressed = true;
        std::cout << "[测试] 已添加临时烟花光源！" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
        tKeyPressed = false;
}

// glfw: 窗口大小改变时调用此回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: 鼠标移动时调用此回调函数
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (!mouseEnabled) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Y 坐标反转（从底部到顶部）

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: 鼠标滚轮滚动时调用此回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
