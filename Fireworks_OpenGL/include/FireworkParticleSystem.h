#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include "Shader.h"

// FireworkParticleSystem - 烟花粒子系统
// 支持发射器（launcher）、上升的礼弹（shell）和爆炸后的子粒子（secondary shell），
// 并包含可选的拖尾（trail）视觉效果。
class FireworkParticleSystem {
public:
    // 支持的烟花类型
    enum class FireworkType { Sphere, Ring, Heart, Star };

    FireworkParticleSystem();
    ~FireworkParticleSystem();

    // 发射一个烟花（上升弹）
    void launch(const glm::vec3& position, FireworkType type, float life, const glm::vec4& color, float size);

    // 更新粒子系统（每帧调用，deltaTime 单位：秒）
    void update(float deltaTime);

    // 渲染粒子（需要先 setViewProj）
    void render();

    // 设置视图和投影矩阵
    void setViewProj(const glm::mat4& view, const glm::mat4& proj);

	// 清理OpenGL资源
    void cleanupGL();

    // === 可调参数（运行时可修改） ===
    // 拖尾参数
    int tailCount = 2;              // 每个活动粒子每帧生成的拖尾粒子数
    float tailStep = 0.3f;         // 拖尾粒子沿反向速度的距离系数
    float tailLife = 0.1f;         // 拖尾粒子的寿命（秒）
    float tailBrightFactor = 1.0f;  // 拖尾粒子的亮度系数

    // 爆炸粒子的初始拖尾参数
    int initTailCount = 3;          // 爆炸粒子初始拖尾数量
    float initTailStep = 0.03f;     // 爆炸粒子初始拖尾步长
    float initTailLife = 1.0f;      // 爆炸粒子初始拖尾寿命
    float initTailBrightFactor = 0.18f; // 爆炸粒子初始拖尾亮度系数

    // 尺寸和物理参数
    float launcherSize = 0.02f; // 上升弹粒子大小（原来0.01f，现为2倍）
    float childSize = 0.02f;    // 爆炸子粒子大小（原来0.015f，现为2倍）
    float gravity = -5.0f;      // 重力加速度（负Y方向）
    float alphaDecayFactor = 0.5f; // 透明度衰减因子（越大衰减越慢）
    float timeScale = 0.3f;     // 时间缩放（1.0=正常，0.5=慢动作）

private:
    struct Particle {
        glm::vec3 position;      // 位置
        glm::vec3 velocity;      // 速度
        glm::vec4 color;         // 颜色（RGBA）
        float life;              // 剩余寿命（秒）
        float size;              // 规格化尺寸（0..1），在顶点着色器中转换为像素大小
        FireworkType type;       // 烟花类型
        bool isTail = false;     // 是否为拖尾粒子，防止递归产生尾迹
    };

    std::vector<Particle> launcherParticles; // 上升粒子
    std::vector<Particle> explosionParticles; // 爆炸粒子
    std::vector<Particle> tailParticles; // 拖尾粒子

    std::vector<Particle> particles; // 渲染时合并所有粒子的容器
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    Shader* shader = nullptr;

    // OpenGL 对象
    GLuint vao = 0;
    GLuint vbo = 0;

    void initGL();
    bool glInited = false;
};
