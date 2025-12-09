#include "InputHandler.h"
#include "Camera.h"
#include "PointLight.h"
#include "FireworkParticleSystem.h"
#include <iostream>



// 外部变量声明（在 main.cpp 中定义）
extern Camera camera;
extern float deltaTime;
extern bool mouseEnabled;
extern float lastX, lastY;
extern bool firstMouse;
extern PointLightManager lightManager;
extern FireworkParticleSystem fireworkSystem;

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

    // 发射烟花 --> 通过三个通道控制颜色
    static bool key1Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed)
    {
        glm::vec3 launchPos = glm::vec3(0.0f, 0.5f, 0.0f); // 场景中心稍上方
        glm::vec3 launchVel = glm::vec3(0.0f, 22.0f, 0.0f); // 竖直向上
        fireworkSystem.launch(
            launchPos,
            FireworkParticleSystem::FireworkType::Sphere,
            1.5f, glm::vec4(1.0f, 1.0f, 0.5f, 1.0f), 0.02f);
        key1Pressed = true;
        std::cout << "[烟花] 发射球形烟花！" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
        key1Pressed = false;

    static bool key2Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed)
    {
        glm::vec3 launchPos = glm::vec3(0.0f, 0.5f, 0.0f);
        glm::vec3 launchVel = glm::vec3(0.0f, 22.0f, 0.0f);
        fireworkSystem.launch(
            launchPos,
            FireworkParticleSystem::FireworkType::Ring,
            1.5f, glm::vec4(0.5f, 1.0f, 1.0f, 1.0f), 0.02f);
        key2Pressed = true;
        std::cout << "[烟花] 发射环形烟花！" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
        key2Pressed = false;

    static bool key3Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !key3Pressed)
    {
        glm::vec3 launchPos = glm::vec3(0.0f, 0.5f, 0.0f);
        glm::vec3 launchVel = glm::vec3(0.0f, 22.0f, 0.0f);
        fireworkSystem.launch(
            launchPos,
            FireworkParticleSystem::FireworkType::Heart,
            1.5f, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f), 0.02f);
        key3Pressed = true;
        std::cout << "[烟花] 发射心形烟花！" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE)
        key3Pressed = false;

    static bool key4Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !key4Pressed)
    {
        glm::vec3 launchPos = glm::vec3(0.0f, 0.5f, 0.0f);
        glm::vec3 launchVel = glm::vec3(0.0f, 22.0f, 0.0f);
        fireworkSystem.launch(
            launchPos,
            FireworkParticleSystem::FireworkType::Star,
            1.5f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.02f);
        key4Pressed = true;
        std::cout << "[烟花] 发射星形烟花！" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE)
        key4Pressed = false;
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
