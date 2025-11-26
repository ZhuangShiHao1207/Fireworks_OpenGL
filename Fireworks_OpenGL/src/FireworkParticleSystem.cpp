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
void FireworkParticleSystem::launch(const glm::vec3& position, const glm::vec3& velocity, FireworkType type) {
    Particle p;
    p.position = position;
    p.velocity = velocity;
    p.color = glm::vec4(1, 1, 0.5, 1);
    p.life = 1.2f;
    // 减小升空主粒子大小，使后续尾迹更协调
    p.size = launcherSize;
    p.type = type;
    particles.push_back(p);
}

// 更新所有粒子状态，并在主粒子生命结束时生成爆炸粒子
void FireworkParticleSystem::update(float deltaTime) {
    float dt = deltaTime * timeScale; // apply time scale for slow motion
    // 预先标记即将爆炸的粒子，避免为它们生成尾迹
    std::vector<bool> willExplode(particles.size(), false);
    for (size_t i = 0; i < particles.size(); ++i) {
        const auto& p = particles[i];
        if (p.size > (launcherSize * 0.6f) && (p.velocity.y <= 0.0f || p.life <= 0.0f)) {
            willExplode[i] = true;
        }
    }

    // 1. 物理更新：位置、速度、寿命、颜色
    std::vector<Particle> tailParticles;
    // History-sampled tail: for each non-tail particle spawn one short-lived tail at
    // the particle's previous position. This produces a clean trailing effect
    // without generating many new tails per frame and avoids tails generating tails.
    for (size_t i = 0; i < particles.size(); ++i) {
        auto& p = particles[i];
        // capture previous position (sample history)
        glm::vec3 prevPos = p.position;

        // 物理更新（对所有粒子生效，包括尾迹）
        if (p.life > 0.0f) {
            // 位置更新
            p.position += p.velocity * dt;
            // 重力影响
            p.velocity += glm::vec3(0, gravity, 0) * dt;
            // 寿命递减
            p.life -= dt;
            // 拖尾/淡出：alpha随life递减，RGB颜色保持
            float alpha = std::max(0.0f, p.life / alphaDecayFactor);
            p.color = glm::vec4(p.color.r, p.color.g, p.color.b, alpha);
        }

        // 仅对非尾粒子且非即将爆炸的粒子生成单帧历史采样尾（避免尾迹再生成尾迹）
            if (!p.isTail && p.life > 0.0f && !willExplode[i]) {
                Particle tail = p;
                // place tail at previous position (history sample)
                tail.position = prevPos;
                // ascending particle tail: much smaller and always white
                tail.size = std::max(0.001f, p.size * 0.25f);
                // alpha fades out quickly for tail
                float tailAlpha = std::max(0.0f, p.life / (tailLife * 1.2f));
                tail.color = glm::vec4(1.0f, 1.0f, 1.0f, tailAlpha);
                tail.isTail = true;
                tail.life = tailLife * 0.4f; // even shorter for ascending
                tail.velocity = glm::vec3(0.0f); // tail does not move
                tailParticles.push_back(tail);
        }
    }

    std::vector<Particle> newParticles;
    std::vector<size_t> toRemove;
    for (size_t i = 0; i < particles.size(); ++i) {
        auto& p = particles[i];
        // 2. 爆炸：主粒子在到达顶点（垂直速度接近或小于 0）时爆炸，
        //    或者在寿命耗尽时爆炸。使用可配置的 launcherSize 作为判断阈值
        //    这样可以避免发射过程中（仍在上升）就提前分散的问题。
        if (p.size > (launcherSize * 0.6f) && (p.velocity.y <= 0.0f || p.life <= 0.0f)) {
            int count = 120; // 爆炸粒子数，提升饱满度
            // 单次爆炸统一颜色
            float colorType = (float)rand() / RAND_MAX;
            glm::vec3 baseColor;
            if (colorType < 0.33f) baseColor = glm::vec3(1, 0.3f + 0.2f * ((float)rand() / RAND_MAX), 0.1f + 0.1f * ((float)rand() / RAND_MAX)); // 红
            else if (colorType < 0.66f) baseColor = glm::vec3(1, 1, 0.3f + 0.2f * ((float)rand() / RAND_MAX)); // 黄
            else baseColor = glm::vec3(0.2f + 0.3f * ((float)rand() / RAND_MAX), 0.4f + 0.3f * ((float)rand() / RAND_MAX), 1); // 蓝
            switch (p.type) {
                case FireworkType::Sphere:
                    // 球面分布，单次爆炸统一色
                    for (int i = 0; i < count; ++i) {
                        float phi = acos(1.0f - 2.0f * (i + 0.5f) / count);
                        float theta = 3.1415926f * (1 + sqrt(5.0f)) * i;
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(
                            sin(phi) * cos(theta),
                            -glm::abs(sin(phi) * sin(theta)),  // 向下
                            cos(phi)
                        ) * (2.5f + 1.5f * ((float)rand() / RAND_MAX));
                        child.color = glm::vec4(baseColor, 1);
                        child.life = 1.2f + 0.5f * ((float)rand() / RAND_MAX);
                        child.size = 0.015f;
                        child.type = p.type;
                        newParticles.push_back(child);
                    }
                    break;

                case FireworkType::Ring:
                    // 环形分布，单次爆炸统一色
                    for (int i = 0; i < count; ++i) {
                        float angle = (float)i / count * 2.0f * 3.14159f;
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(
                            cos(angle),
                            -(0.2f + 0.8f * ((float)rand() / RAND_MAX)),  // 向下
                            sin(angle)
                        ) * (3.0f + 1.0f * ((float)rand() / RAND_MAX));
                        child.color = glm::vec4(baseColor, 1);
                        child.life = 1.5f + 0.3f * ((float)rand() / RAND_MAX);
                        child.size = 0.015f;
                        child.type = p.type;
                        newParticles.push_back(child);
                    }
                    break;

                case FireworkType::Heart:
                    // 心形分布，单次爆炸统一色
                    for (int i = 0; i < count; ++i) {
                        float t = (float)i / count * 2.0f * 3.14159f;
                        float x = 16 * sin(t) * sin(t) * sin(t);
                        float y = 13 * cos(t) - 5 * cos(2*t) - 2 * cos(3*t) - cos(4*t);
                        y = -glm::abs(y);  // 向下
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(x, y, 0) * 0.1f + glm::vec3(
                            ((float)rand() / RAND_MAX - 0.5f) * 0.5f,
                            ((float)rand() / RAND_MAX - 0.5f) * 0.5f,
                            ((float)rand() / RAND_MAX - 0.5f) * 0.5f
                        );
                        child.color = glm::vec4(baseColor, 1);
                        child.life = 1.8f + 0.4f * ((float)rand() / RAND_MAX);
                        child.size = 0.015f;
                        child.type = p.type;
                        newParticles.push_back(child);
                    }
                    break;

                case FireworkType::Star:
                    // 星形分布，单次爆炸统一色
                    for (int i = 0; i < count; ++i) {
                        float angle = (float)i / count * 2.0f * 3.14159f;
                        float radius = (i % 2 == 0) ? 3.5f : 2.0f;
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(
                            cos(angle) * radius,
                            -(1.0f + 0.5f * ((float)rand() / RAND_MAX)),  // 向下
                            sin(angle) * radius
                        );
                        child.color = glm::vec4(baseColor, 1);
                        child.life = 1.6f + 0.4f * ((float)rand() / RAND_MAX);
                        child.size = 0.015f;
                        child.type = p.type;
                        newParticles.push_back(child);
                    }
                    break;
            }
            toRemove.push_back(i);
        }
    }
    // 删除爆炸的上升粒子
    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
        particles.erase(particles.begin() + *it);
    }
    // 为新生成的爆炸粒子添加初始拖尾（让爆炸瞬间也带上拖尾）
    std::vector<Particle> initialTails;
    for (const auto& child : newParticles) {
        for (int t = 1; t <= initTailCount; ++t) {
            Particle tail = child;
            tail.position = child.position - child.velocity * (float)t * initTailStep;
            tail.size = std::max(0.004f, child.size * (1.0f - 0.2f * t));
            // 初始尾全白，alpha 衰减
            tail.color = glm::vec4(1.0f, 1.0f, 1.0f, child.color.a * (1.0f - 0.20f * t));
            tail.isTail = true;
            tail.life = initTailLife;
            initialTails.push_back(tail);
        }
    }

    // 合并拖尾粒子和新粒子
    particles.insert(particles.end(), tailParticles.begin(), tailParticles.end());
    particles.insert(particles.end(), initialTails.begin(), initialTails.end());
    particles.insert(particles.end(), newParticles.begin(), newParticles.end());

    // 粒子消失条件：生命耗尽或落地
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p) {
        return (p.life <= 0.0f) || (p.position.y < 0.0f);
    }), particles.end());
}

void FireworkParticleSystem::render() {
    if (!glInited) initGL();
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
