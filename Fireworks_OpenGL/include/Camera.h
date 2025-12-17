#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

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

    // Cinematic trajectory mode
    bool CinematicMode;
    float CinematicTime;
    glm::vec3 CinematicInitialPos;
    glm::vec3 CinematicTargetCenter;

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
          OrbitCenter(glm::vec3(0.0f, 5.0f, 0.0f)),
          CinematicMode(false),
          CinematicTime(0.0f),
          CinematicInitialPos(glm::vec3(0.0f, 10.0f, 20.0f)),
          CinematicTargetCenter(glm::vec3(0.0f, 3.0f, 0.0f)) {
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
          OrbitCenter(glm::vec3(0.0f, 5.0f, 0.0f)),
          CinematicMode(false),
          CinematicTime(0.0f),
          CinematicInitialPos(glm::vec3(0.0f, 10.0f, 20.0f)),
          CinematicTargetCenter(glm::vec3(0.0f, 3.0f, 0.0f)) {
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
        if (OrbitMode || CinematicMode) return; // 轨道模式和电影模式下禁止移动

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
        if (OrbitMode || CinematicMode) return; // 轨道模式和电影模式下禁止鼠标控制

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
            CinematicMode = false; // 启用轨道模式时禁用电影模式
        }
    }

    // 切换电影轨迹模式
    void ToggleCinematicMode() {
        CinematicMode = !CinematicMode;
        if (CinematicMode) {
            CinematicTime = 0.0f;
            OrbitMode = false; // 启用电影模式时禁用轨道模式
            // 保存初始位置以便在电影结束后恢复
            CinematicInitialPos = Position;
        }
    }

    // 更新电影轨迹
    void UpdateCinematicMode(float deltaTime) {
        if (!CinematicMode) return;

        CinematicTime += deltaTime;
        float t = CinematicTime;

        // 第一阶段 (0-3.5s): 高空俯瞰，缓慢接近 - 向右偏移，更前更低
        if (t < 3.5f) {
            float phase = t / 3.5f;
            // 从高处和远处开始，缓慢接近
            float startDist = 27.0f;  // 起始距离
            float endDist = 17.0f;    // 结束距离
            float dist = glm::mix(startDist, endDist, phase);
            
			// 摄像头起始位置，x表示左右，y表示高度，z表示前后距离
			Position.x = 4.0f;  // +表示向右偏移
			Position.y = glm::mix(7.0f, 4.0f, phase); // 第一个参数是起始高度，第二个参数是结束高度
			Position.z = dist; 
            
            // 稍微向下的俯仰角以查看岛屿
            glm::vec3 lookTarget = CinematicTargetCenter + glm::vec3(0.0f, 1.0f, 0.0f);
            Front = glm::normalize(lookTarget - Position);
        }
        // 第二阶段 (3.5-8.5s): 绕岛屿轨道飞行并添加电影晃动
        else if (t < 8.5f) {
            float phase = (t - 3.5f) / 5.0f; // 5秒完成旋转
            float angle = phase * 160.0f; // 减慢旋转速度（从180度改为160度）
            float angleRad = glm::radians(angle);
            
            // 更大的圆形轨道，向右偏移
            float orbitRadius = 22.0f; // 半径
            Position.x = 4.0f + orbitRadius * sin(angleRad); // 基础向右偏移 + 轨道
            Position.z = orbitRadius * cos(angleRad);
            Position.y = 5.5f; // 稍微降低高度
            
            // 添加细微的电影晃动
            float shakeFreq = 2.5f;
            float shakeAmp = 0.12f;
            Position.x += sin(t * shakeFreq) * shakeAmp;
            Position.y += cos(t * shakeFreq * 1.3f) * shakeAmp * 0.5f;
            
            // 注视岛屿中心
            Front = glm::normalize(CinematicTargetCenter - Position);
        }
        // 第三阶段 (8.5-11s): 返回并下降到烟花观赏点
        else if (t < 11.0f) {
            float phase = (t - 8.5f) / 2.5f; 
            phase = smoothstep(0.0f, 1.0f, phase); // 平滑过渡
            
            // 从轨道位置到烟花观赏点，降低高度，拉远距离
            glm::vec3 startPos = glm::vec3(4.0f, 6.5f, 20.0f);
            glm::vec3 endPos = glm::vec3(4.0f, 3.0f, 20.0f);
            Position = glm::mix(startPos, endPos, phase);
            
            // 渐渐仰视天空
            float startPitch = -8.0f;
            float endPitch = 15.0f;
            float pitch = glm::mix(startPitch, endPitch, phase);
            
            // 计算带有俯仰的前向量
            Front.x = 0.0f;
            Front.y = sin(glm::radians(pitch));
            Front.z = -cos(glm::radians(pitch));
            Front = glm::normalize(Front);
        }
        // 超过11秒后，停止电影模式
        else {
            CinematicMode = false;
            std::cout << "[Camera] Cinematic trajectory completed!" << std::endl;
        }
        
        // 更新 Right 和 Up 向量
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
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

    // 获取电影模式的淡入淡出透明度（用于阶段过渡）
    float GetCinematicFadeAlpha() const {
        if (!CinematicMode) return 1.0f;
        
        float t = CinematicTime;
        float fadeDuration = 0.5f; // 淡入淡出持续时间
        
        // 第一阶段结束时淡出 (3.5s附近)
        if (t > 3.0f && t < 3.5f) {
            return 1.0f - ((t - 3.0f) / fadeDuration);
        }
        // 第二阶段开始时淡入 (3.5s-4.0s)
        else if (t >= 3.5f && t < 4.0f) {
            return (t - 3.5f) / fadeDuration;
        }
        // 第二阶段结束时淡出 (8.0s-8.5s)
        else if (t > 8.0f && t < 8.5f) {
            return 1.0f - ((t - 8.0f) / fadeDuration);
        }
        // 第三阶段开始时淡入 (8.5s-9.0s)
        else if (t >= 8.5f && t < 9.0f) {
            return (t - 8.5f) / fadeDuration;
        }
        
        return 1.0f; // 其他时间完全不透明
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
    
    // Smoothstep function for smooth interpolation
    float smoothstep(float edge0, float edge1, float x) {
        x = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    }
};

#endif
