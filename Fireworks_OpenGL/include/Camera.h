#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// 摄像机移动方向枚举
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// 默认摄像机参数
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

/**
 * Camera 类
 * 处理摄像机输入并计算相应的欧拉角、向量和矩阵
 * 支持自由移动模式和轨道环绕模式
 */
class Camera {
public:
    // 摄像机属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // 欧拉角
    float Yaw;
    float Pitch;

    // 摄像机选项
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // 轨道模式参数
    bool OrbitMode;
    float OrbitRadius;
    float OrbitSpeed;
    float OrbitAngle;
    glm::vec3 OrbitCenter;

    // 使用向量的构造函数
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW,
           float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY),
          Zoom(ZOOM),
          OrbitMode(false),
          OrbitRadius(10.0f),
          OrbitSpeed(0.5f),
          OrbitAngle(0.0f),
          OrbitCenter(glm::vec3(0.0f, 5.0f, 0.0f)) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 使用标量值的构造函数
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY),
          Zoom(ZOOM),
          OrbitMode(false),
          OrbitRadius(10.0f),
          OrbitSpeed(0.5f),
          OrbitAngle(0.0f),
          OrbitCenter(glm::vec3(0.0f, 5.0f, 0.0f)) {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 返回使用欧拉角和 LookAt 矩阵计算的视图矩阵
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // 处理键盘输入
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        if (OrbitMode) return; // 轨道模式下禁止移动

        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += Up * velocity;
        if (direction == DOWN)
            Position -= Up * velocity;
    }

    // 处理鼠标移动输入
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        if (OrbitMode) return; // 轨道模式下禁止鼠标控制

        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // 确保俯仰角不会超出范围，防止屏幕翻转
        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // 使用更新的欧拉角更新 Front、Right 和 Up 向量
        updateCameraVectors();
    }

    // 处理鼠标滚轮输入
    void ProcessMouseScroll(float yoffset) {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    // 切换轨道模式
    void ToggleOrbitMode() {
        OrbitMode = !OrbitMode;
        if (OrbitMode) {
            OrbitAngle = 0.0f;
        }
    }

    // 更新轨道模式摄像机
    void UpdateOrbitMode(float deltaTime) {
        if (!OrbitMode) return;

        OrbitAngle += OrbitSpeed * deltaTime;
        if (OrbitAngle > 360.0f) OrbitAngle -= 360.0f;

        // 计算轨道上的新摄像机位置
        float angleRad = glm::radians(OrbitAngle);
        Position.x = OrbitCenter.x + OrbitRadius * cos(angleRad);
        Position.z = OrbitCenter.z + OrbitRadius * sin(angleRad);
        Position.y = OrbitCenter.y + 5.0f; // 略微抬高

        // 注视轨道中心
        Front = glm::normalize(OrbitCenter - Position);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }

    // 聚焦到特定点
    void FocusOn(glm::vec3 target) {
        Front = glm::normalize(target - Position);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));

        // 根据新的 Front 向量更新 Yaw 和 Pitch
        Pitch = glm::degrees(asin(Front.y));
        Yaw = glm::degrees(atan2(Front.z, Front.x));
    }

    // 设置轨道中心
    void SetOrbitCenter(glm::vec3 center) {
        OrbitCenter = center;
    }

private:
    // 从摄像机的（更新的）欧拉角计算 front 向量
    void updateCameraVectors() {
        // 计算新的 Front 向量
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        // 重新计算 Right 和 Up 向量
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif
