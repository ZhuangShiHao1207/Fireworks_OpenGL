#include "InputHandler.h"
#include "Camera.h"
#include "PointLight.h"
#include "FireworkParticleSystem.h"
#include "PostProcessor.h"
#include <iostream>
#include <UIManager.h>

// 外部变量声明（在 main.cpp 中定义）
extern Camera camera;
extern float deltaTime;
extern bool mouseEnabled;
extern float lastX, lastY;
extern bool firstMouse;
extern bool sceneLightsEnabled;
extern PointLightManager lightManager;
extern FireworkParticleSystem fireworkSystem;
extern PostProcessor* postProcessor;
extern int g_fireworkKeyPressCount;
extern UIManager* uiManager;

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
        if (uiManager) {
            uiManager->SetMouseEnabled(mouseEnabled);
        }
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
        if (uiManager) {
            uiManager->SetSceneLightsEnabled(sceneLightsEnabled);
        }
        std::cout << "[Lights] Scene lights: " << (sceneLightsEnabled ? "ON" : "OFF") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
        lKeyPressed = false;

    // 切换控制提示时
    static bool hKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hKeyPressed) {
        if (uiManager) {
            uiManager->ToggleControlHints();
        }
        hKeyPressed = true;
        std::cout << "[UI] Toggled control hints" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
        hKeyPressed = false;
    }

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

        // 更新UI状态
        if (uiManager) {
            uiManager->SetFireworkType(1);
            uiManager->IncrementFireworkCount();
        }

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

        // 更新UI状态
        if (uiManager) {
            uiManager->SetFireworkType(2);
            uiManager->IncrementFireworkCount();
        }

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

        // 更新UI状态
        if (uiManager) {
            uiManager->SetFireworkType(3);
            uiManager->IncrementFireworkCount();
        }

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

        // 更新UI状态
        if (uiManager) {
            uiManager->SetFireworkType(4);
            uiManager->IncrementFireworkCount();
        }

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

        // 更新UI状态
        if (uiManager) {
            uiManager->SetFireworkType(5);
            uiManager->IncrementFireworkCount();
        }

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

        // 更新UI状态
        if (uiManager) {
            uiManager->SetFireworkType(6);
            uiManager->IncrementFireworkCount();
        }

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
    
    // 如果 PostProcessor 存在，重新创建以匹配新的窗口大小
    if (postProcessor) {
        delete postProcessor;
        postProcessor = new PostProcessor(width, height);
        std::cout << "[Resize] PostProcessor recreated with size: " << width << "x" << height << std::endl;
    }
    // 如果 uiManager 存在，更新UI窗口以匹配新的窗口大小
    if (uiManager) {
        uiManager->UpdateScreenSize(width, height);
        std::cout << "[Resize] UI updated with size: " << width << "x" << height << std::endl;
    }
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

// glfw: 鼠标点击时调用此回调函数
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // 只在鼠标锁定模式下发射烟花
    if (!mouseEnabled) return;

    // 检查是否是左键按下
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // 获取当前鼠标位置（在锁定模式下，鼠标位置可能为0,0，我们需要使用摄像机方向）
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // 将屏幕坐标转换为世界坐标（需要深度缓冲信息，这里简化处理）
        // 使用摄像机前方的位置作为发射基准点

        // 从摄像机位置向前发射烟花
        glm::vec3 launchPosition = camera.Position + camera.Front * 5.0f; // 摄像机前方5个单位
        launchPosition.y = 0.5f; // 设置在地面上方一点

        // 根据当前选择的烟花类型发射
        static int currentFireworkType = 1; // 默认球形烟花

        // 更新UI中的烟花计数
        if (uiManager) {
            uiManager->IncrementFireworkCount();
        }

        // 发射烟花
        switch (currentFireworkType) {
        case 1:
            fireworkSystem.launch(
                launchPosition,
                FireworkParticleSystem::FireworkType::Sphere,
                1.5f,
                glm::vec4(1.0f, 1.0f, 0.5f, 1.0f),
                fireworkSystem.launcherSize
            );
            std::cout << "[Mouse] Launch Sphere firework at ("
                << launchPosition.x << ", " << launchPosition.y << ", " << launchPosition.z << ")" << std::endl;
            break;

        case 2:
            fireworkSystem.launch(
                launchPosition,
                FireworkParticleSystem::FireworkType::Ring,
                1.5f,
                glm::vec4(0.5f, 1.0f, 1.0f, 1.0f),
                fireworkSystem.launcherSize
            );
            std::cout << "[Mouse] Launch Ring firework" << std::endl;
            break;

        case 3:
            fireworkSystem.launch(
                launchPosition,
                FireworkParticleSystem::FireworkType::Heart,
                1.5f,
                glm::vec4(1.0f, 0.5f, 1.0f, 1.0f),
                fireworkSystem.launcherSize
            );
            std::cout << "[Mouse] Launch Heart firework" << std::endl;
            break;

        case 4:
            fireworkSystem.launch(
                launchPosition,
                FireworkParticleSystem::FireworkType::MultiLayer,
                1.5f,
                glm::vec4(0.3f, 0.8f, 1.0f, 1.0f),
                fireworkSystem.launcherSize
            );
            std::cout << "[Mouse] Launch MultiLayer firework" << std::endl;
            break;

        case 5:
            fireworkSystem.launch(
                launchPosition,
                FireworkParticleSystem::FireworkType::Spiral,
                1.5f,
                glm::vec4(1.0f, 0.8f, 0.2f, 1.0f),
                fireworkSystem.launcherSize
            );
            std::cout << "[Mouse] Launch Spiral firework" << std::endl;
            break;

        case 6:
            fireworkSystem.launch(
                launchPosition,
                FireworkParticleSystem::FireworkType::DoubleExplosion,
                1.5f,
                glm::vec4(0.8f, 0.3f, 1.0f, 1.0f),
                fireworkSystem.launcherSize
            );
            std::cout << "[Mouse] Launch DoubleExplosion firework" << std::endl;
            break;
        }
    }
}

// glfw: 鼠标滚轮滚动时调用此回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 鼠标位置辅助函数
glm::vec3 GetMouseWorldPosition(GLFWwindow* window, Camera& camera, float distance = 10.0f)
{
    // 获取鼠标位置
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // 将屏幕坐标转换为归一化设备坐标
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float x = (2.0f * xpos) / width - 1.0f;
    float y = 1.0f - (2.0f * ypos) / height;

    // 创建射线
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    // 需要投影矩阵和视图矩阵来转换到世界空间
    // 这里简化：使用摄像机方向作为发射方向
    return camera.Position + camera.Front * distance;
}