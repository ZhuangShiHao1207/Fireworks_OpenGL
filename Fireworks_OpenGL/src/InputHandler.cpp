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
extern bool sceneLightsEnabled;
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

    // 切换场景灯光
    static bool lKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !lKeyPressed)
    {
        sceneLightsEnabled = !sceneLightsEnabled;
        lKeyPressed = true;
        std::cout << "[Lights] Scene lights: " << (sceneLightsEnabled ? "ON" : "OFF") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
        lKeyPressed = false;

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
        std::cout << "[Test] Added temporary light!" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
        tKeyPressed = false;

    // 发射烟花 --> 通过三个通道控制颜色
    static bool key1Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed)
    {
        glm::vec3 launchPos = glm::vec3(0.0f, 0.5f, 0.0f);
        fireworkSystem.launch(
            launchPos,
            FireworkParticleSystem::FireworkType::Sphere,
            1.5f, glm::vec4(1.0f, 1.0f, 0.5f, 1.0f), fireworkSystem.launcherSize);
        key1Pressed = true;
        std::cout << "[Firework] Launch Sphere!" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
        key1Pressed = false;

    static bool key2Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed)
    {
        glm::vec3 launchPos = glm::vec3(0.0f, 0.5f, 0.0f);
        fireworkSystem.launch(
            launchPos,
            FireworkParticleSystem::FireworkType::Ring,
            1.5f, glm::vec4(0.5f, 1.0f, 1.0f, 1.0f), fireworkSystem.launcherSize);
        key2Pressed = true;
        std::cout << "[Firework] Launch Ring!" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
        key2Pressed = false;

    static bool key3Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !key3Pressed)
    {
        glm::vec3 launchPos = glm::vec3(0.0f, 0.5f, 0.0f);
        fireworkSystem.launch(
            launchPos,
            FireworkParticleSystem::FireworkType::Heart,
            1.5f, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f), fireworkSystem.launcherSize);
        key3Pressed = true;
        std::cout << "[Firework] Launch Heart!" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE)
        key3Pressed = false;

    static bool key4Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !key4Pressed)
    {
        glm::vec3 launchPos = glm::vec3(0.0f, 0.5f, 0.0f);
        fireworkSystem.launch(
            launchPos,
            FireworkParticleSystem::FireworkType::MultiLayer,
            1.5f, glm::vec4(0.3f, 0.8f, 1.0f, 1.0f), fireworkSystem.launcherSize);
        key4Pressed = true;
        std::cout << "[Firework] Launch MultiLayer!" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE)
        key4Pressed = false;

    static bool key5Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && !key5Pressed)
    {
        glm::vec3 launchPos = glm::vec3(0.0f, 0.5f, 0.0f);
        fireworkSystem.launch(
            launchPos,
            FireworkParticleSystem::FireworkType::Spiral,
            1.5f, glm::vec4(1.0f, 0.8f, 0.2f, 1.0f), fireworkSystem.launcherSize);
        key5Pressed = true;
        std::cout << "[Firework] Launch Spiral!" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE)
        key5Pressed = false;

    static bool key6Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS && !key6Pressed)
    {
        glm::vec3 launchPos = glm::vec3(0.0f, 0.5f, 0.0f);
        fireworkSystem.launch(
            launchPos,
            FireworkParticleSystem::FireworkType::DoubleExplosion,
            1.5f, glm::vec4(0.8f, 0.3f, 1.0f, 1.0f), fireworkSystem.launcherSize);
        key6Pressed = true;
        std::cout << "[Firework] Launch DoubleExplosion!" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE)
        key6Pressed = false;

    // 运行测试序列
    static bool key0Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && !key0Pressed)
    {
        std::cout << "[Test] Start firework test sequence (auto-launch every 3s)" << std::endl;
        key0Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE)
        key0Pressed = false;
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
