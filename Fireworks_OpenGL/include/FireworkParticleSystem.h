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
    void launch(const glm::vec3& position, const glm::vec3& velocity, FireworkType type);

    // 更新粒子系统（每帧调用，deltaTime 单位：秒）
    void update(float deltaTime);

    // 渲染粒子（需要先 setViewProj）
    void render();

    // 设置视图和投影矩阵
    void setViewProj(const glm::mat4& view, const glm::mat4& proj);

    // === Configurable parameters (tweak at runtime) ===
    // Tail (trail) parameters for moving particles
    int tailCount = 3;              // number of tail particles generated per active particle per frame
    float tailStep = 0.03f;         // distance multiplier for tail placement along reverse velocity
    float tailLife = 0.1f;         // lifetime (seconds) of generated tail particles (reduced to prevent lingering after explosion)
    float tailBrightFactor = 1.0f;  // per-step brightness multiplier increment for tail particles (increased for brightness)

    // Initial tails generated for explosion particles
    int initTailCount = 3;
    float initTailStep = 0.03f;
    float initTailLife = 1.0f;       // reduced to prevent lingering
    float initTailBrightFactor = 0.18f;

    // Sizes and physics
    float launcherSize = 0.04f; // size of the launcher (ascending shell)
    float childSize = 0.015f;   // default size for explosion child particles
    float gravity = -0.8f;      // gravity acceleration (negative Y)
    float alphaDecayFactor = 0.5f; // divisor for alpha decay (larger -> slower decay)
    float timeScale = 0.3f;     // time scale for slow motion (1.0 = normal, 0.5 = half speed)

private:
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec4 color;   // rgba
        float life;        // 剩余寿命（秒）
        float size;        // 规格化尺寸（0..1），在顶点着色器中转换为像素大小
        FireworkType type;
        bool isTail = false; // 标记为拖尾粒子，防止递归产生尾迹
    };

    std::vector<Particle> particles;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    Shader* shader = nullptr;

    // OpenGL 对象
    GLuint vao = 0;
    GLuint vbo = 0;

    void initGL();
    void cleanupGL();
    bool glInited = false;
};
