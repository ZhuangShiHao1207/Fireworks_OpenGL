#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include "Shader.h"
#include "PointLight.h"

// FireworkParticleSystem - 烟花粒子系统
// 支持多种烟花类型、颜色渐变、二次爆炸、拖尾效果
class FireworkParticleSystem {
public:
    // 支持的烟花类型
    enum class FireworkType {
        Sphere,          // 球形烟花
        Ring,            // 环形烟花
        MultiLayer,      // 多层烟花
        Spiral,          // 螺旋烟花
        Heart,           // 心形烟花
        DoubleExplosion  // 二次爆炸烟花
    };

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

    // 设置光源管理器指针（用于烟花爆炸时添加点光源）
    void setLightManager(PointLightManager* manager);

    // 测试方法：依次发射各种类型烟花
    void runTest(float currentTime);

    // 拖尾参数
    float tailLife = 0.02f;         // 拖尾粒子的寿命（秒）
    float tailInterval = 0.016f;    // 拖尾生成间隔（秒）
    float tailAlpha = 0.5f;         // 拖尾透明度系数

    // 尺寸和物理参数
    float launcherSize = 0.05f; // 上升弹粒子大小
    float childSize = 0.05f;     // 爆炸子粒子大小
    float gravity = -5.0f;      // 重力加速度（负Y方向）
    float timeScale = 0.2f;     // 时间缩放（1.0=正常，0.5=慢动作）

private:
    struct Particle {
        glm::vec3 position;      // 位置
        glm::vec3 velocity;      // 速度
        glm::vec4 color;         // 颜色（RGBA）
        glm::vec4 initialColor;  // 初始颜色（用于颜色渐变）
        float life;              // 剩余寿命（秒）
        float maxLife;           // 最大寿命（用于计算生命周期比例）
        float size;              // 规格化尺寸（0..1），在顶点着色器中转换为像素大小
        FireworkType type;       // 烟花类型
        bool isTail = false;     // 是否为拖尾粒子，防止递归产生尾迹
        bool canExplodeAgain = false; // 是否可以二次爆炸
        float rotationAngle = 0.0f;   // 旋转角度（用于螺旋烟花）
        float tailTimer = 0.0f;       // 拖尾生成计时器
    };

    std::vector<Particle> launcherParticles; // 上升粒子
    std::vector<Particle> explosionParticles; // 爆炸粒子
    std::vector<Particle> tailParticles; // 拖尾粒子

    std::vector<Particle> particles; // 渲染时合并所有粒子的容器
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    Shader* shader = nullptr;
    PointLightManager* lightManager = nullptr;

    // OpenGL 对象
    GLuint vao = 0;
    GLuint vbo = 0;

    void initGL();
    void cleanupGL();
    bool glInited = false;

    // 辅助方法
    void createExplosion(const Particle& source, bool isSecondary = false);
    glm::vec4 calculateColorGradient(const Particle& p) const;
    void generateSphereParticles(const glm::vec3& center, const glm::vec4& color, int count);
    void generateRingParticles(const glm::vec3& center, const glm::vec4& color, int count);
    void generateMultiLayerParticles(const glm::vec3& center, const glm::vec4& color, int count);
    void generateSpiralParticles(const glm::vec3& center, const glm::vec4& color, int count);
    void generateHeartParticles(const glm::vec3& center, const glm::vec4& color, int count);
};
