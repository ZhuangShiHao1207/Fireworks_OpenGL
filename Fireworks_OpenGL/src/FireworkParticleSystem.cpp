#include "FireworkParticleSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <algorithm>
#include <glad/glad.h>

FireworkParticleSystem::FireworkParticleSystem() {
    vao = 0;
    vbo = 0;
    glInited = false;
    shader = nullptr;
}

FireworkParticleSystem::~FireworkParticleSystem() {
    if (shader) {
        delete shader;
        shader = nullptr;
    }
    if (glInited) {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
}

void FireworkParticleSystem::initGL() {
    if (glInited) return;

    // 初始化着色器
    if (!shader) {
        shader = new Shader("assets/shaders/firework.vs", "assets/shaders/firework.fs");
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color));
    glEnableVertexAttribArray(1);
    // 每个粒子的点大小（需要在顶点着色器中读取并使用）
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, size));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    glInited = true;
}

// 发射一个主烟花粒子（上升弹）
void FireworkParticleSystem::launch(const glm::vec3& position, FireworkType type, float life, const glm::vec4& color, float size) {
    // 发射一个主烟花粒子（上升弹）
    // 参数说明：
    //   position: 初始位置
    //   type: 烟花类型
    //   life: 上升寿命
    //   color: 主粒子颜色
    //   size: 主粒子大小

    // 固定上升速度（如：向上 8.0f）
    glm::vec3 fixedVelocity(0.0f, 12.0f, 0.0f);

    Particle p;
    // 不使用变量存储浮点数再用会报错 --> 默认是double，类型不匹配？
    float scale = 2.0;

    p.position = position; // 初始位置
    p.velocity = fixedVelocity; // 固定上升速度
	p.color = color * scale; // 指定颜色 --> 手动提高亮度（本身并没有变亮）
    p.life = life; // 指定寿命
    p.size = size; // 指定大小
    p.type = type; // 烟花类型
    p.isTail = false;
    launcherParticles.push_back(p);
}


void FireworkParticleSystem::update(float deltaTime) {
    float dt = deltaTime * timeScale; // apply time scale for slow motion

    // 1. 更新上升粒子（主粒子）
    // 每帧：
    //   - 更新主粒子物理状态（位置、速度、寿命、颜色）
    //   - 生成一个拖尾粒子，位置为主粒子的上一帧位置，属性与主粒子类似但寿命短
    //   - 检查主粒子是否到达爆炸条件，若满足则生成爆炸粒子
    std::vector<Particle> toExplode; // 保存要爆炸的主粒子
    std::vector<size_t> launcherToRemove;
    for (size_t i = 0; i < launcherParticles.size(); ++i) {
        auto& p = launcherParticles[i];
        glm::vec3 prevPos = p.position;

        // 物理更新
        if (p.life > 0.0f) {
            p.position += p.velocity * dt;
            p.velocity += glm::vec3(0, gravity, 0) * dt;
            p.life -= dt;
            // 保持粒子不透明，直到life耗尽
            p.color = glm::vec4(p.color.r, p.color.g, p.color.b, 1.0f);
        }

        // 拖尾生成：每帧为主粒子生成一个短寿命拖尾，拖尾生成后在原地，逐渐变小、变亮，每次只生成一个
        if (!p.isTail && p.life > 0.0f) {
            Particle tail = p;
            tail.position = prevPos; // 拖尾固定在上一帧主粒子位置
            // 拖尾初始属性与主粒子完全一致
            // 颜色、大小、透明度都与主粒子相同
            tail.isTail = true;
            tail.life = 0.05f;
            tail.velocity = glm::vec3(0.0f); // 拖尾静止
            tailParticles.push_back(tail);
        }

        // 爆炸条件：主粒子尺寸大于阈值且速度y<=0或寿命耗尽
        if (p.size > (launcherSize * 0.6f) && (p.velocity.y <= 0.0f || p.life <= 0.0f)) {
            toExplode.push_back(p); // 保存要爆炸的主粒子
            launcherToRemove.push_back(i);
        }
    }
    // 先移除爆炸的上升粒子
    for (auto it = launcherToRemove.rbegin(); it != launcherToRemove.rend(); ++it) {
        launcherParticles.erase(launcherParticles.begin() + *it);
    }
    // 再生成爆炸粒子
    std::vector<Particle> newExplosionParticles;
    for (const auto& p : toExplode) {
        // 生成爆炸粒子，形状由type决定
        int count = 120;
        // 使用主粒子的颜色作为爆炸粒子颜色
        glm::vec4 explosionColor = p.color;
        // 所有类型都用立体均匀球面爆炸
        for (int j = 0; j < count; ++j) {
            float u = (float)j / (float)count;
            float v = (float)rand() / (float)RAND_MAX;
            float theta = u * 2.0f * 3.1415926f; // azimuth
            float phi = acos(2.0f * v - 1.0f);    // inclination
            float r = 2.5f + 1.5f * ((float)rand() / RAND_MAX);
            Particle child;
            child.position = p.position;
            child.velocity = glm::vec3(
                sin(phi) * cos(theta),
                glm::abs(sin(phi) * sin(theta)),  // 确保向上
                cos(phi)
            ) * r;
            child.color = explosionColor; // 使用主粒子的颜色
            child.life = 1.2f + 0.5f * ((float)rand() / RAND_MAX);
            child.size = childSize;
            child.type = p.type;
            child.isTail = false;
            newExplosionParticles.push_back(child);
        }
    }
    // 添加新爆炸粒子
    explosionParticles.insert(explosionParticles.end(), newExplosionParticles.begin(), newExplosionParticles.end());


    // 2. 更新爆炸粒子
    for (auto& p : explosionParticles) {
        if (p.life > 0.0f) {
            glm::vec3 prevPos = p.position;
            p.position += p.velocity * dt;
            p.velocity += glm::vec3(0, gravity, 0) * dt;
            p.life -= dt;
            // 保持粒子不透明，直到life耗尽
            p.color = glm::vec4(p.color.r, p.color.g, p.color.b, 1.0f);

            // 爆炸粒子动态拖尾：每帧生成一个短寿命拖尾，固定在上一帧位置
            Particle tail = p;
            tail.position = prevPos;
            tail.isTail = true;
            tail.life = 0.05f;
            tail.velocity = glm::vec3(0.0f);
            tailParticles.push_back(tail);
        }
    }

    // 拖尾粒子更新：逐渐变小、变白色
    for (auto& tail : tailParticles) {
        if (tail.life > 0.0f) {
            float t = 1.0f - (tail.life / 0.1f); // 0~1
            tail.size = std::max(0.004f, tail.size * (1.0f - t * 0.7f)); // 逐渐变小
            tail.life -= dt;
        }
    }
    launcherParticles.erase(std::remove_if(launcherParticles.begin(), launcherParticles.end(), [](const Particle& p) {
        return (p.life <= 0.0f) || (p.position.y < 0.0f);
    }), launcherParticles.end());
    explosionParticles.erase(std::remove_if(explosionParticles.begin(), explosionParticles.end(), [](const Particle& p) {
        return (p.life <= 0.0f) || (p.position.y < 0.0f);
    }), explosionParticles.end());
    tailParticles.erase(std::remove_if(tailParticles.begin(), tailParticles.end(), [](const Particle& p) {
        return (p.life <= 0.0f) || (p.position.y < 0.0f);
    }), tailParticles.end());
}

void FireworkParticleSystem::render() {
    if (!glInited) initGL();

    // 合并所有粒子到 particles 用于渲染
    particles.clear();
    particles.insert(particles.end(), launcherParticles.begin(), launcherParticles.end());
    particles.insert(particles.end(), explosionParticles.begin(), explosionParticles.end());
    particles.insert(particles.end(), tailParticles.begin(), tailParticles.end());

    if (particles.empty() || !shader) return;

    // 调试输出粒子数量
    std::cout << "[Firework] Particles: " << particles.size() << std::endl;

    shader->use();
    shader->setMat4("view", viewMatrix);
    shader->setMat4("projection", projMatrix);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_PROGRAM_POINT_SIZE); // 确保点大小生效

    glDrawArrays(GL_POINTS, 0, (GLsizei)particles.size());

    glDisable(GL_PROGRAM_POINT_SIZE);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void FireworkParticleSystem::setViewProj(const glm::mat4& view, const glm::mat4& proj) {
    viewMatrix = view;
    projMatrix = proj;
}

void FireworkParticleSystem::cleanupGL() {
    if (!glInited) return; // 如果GL未初始化，直接返回

    // 删除顶点缓冲对象
    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    // 删除顶点数组对象
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    // 删除 Shader 对象（假设你用 new Shader 创建）
    if (shader) {
        delete shader;
        shader = nullptr;
    }

    glInited = false;
}