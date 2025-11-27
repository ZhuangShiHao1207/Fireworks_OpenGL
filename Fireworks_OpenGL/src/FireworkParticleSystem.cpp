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
    // 上升主粒子大小（2倍）
    p.size = launcherSize;
    p.type = type;
    launcherParticles.push_back(p);
}

// 更新所有粒子状态，并在主粒子生命结束时生成爆炸粒子
void FireworkParticleSystem::update(float deltaTime) {
    float dt = deltaTime * timeScale; // apply time scale for slow motion

    // 1. 更新上升粒子
    std::vector<Particle> newExplosionParticles;
    std::vector<size_t> launcherToRemove;
    for (size_t i = 0; i < launcherParticles.size(); ++i) {
        auto& p = launcherParticles[i];
        glm::vec3 prevPos = p.position;

        // 物理更新
        if (p.life > 0.0f) {
            p.position += p.velocity * dt;
            p.velocity += glm::vec3(0, gravity, 0) * dt;
            p.life -= dt;
            float alpha = std::max(0.0f, p.life / alphaDecayFactor);
            p.color = glm::vec4(p.color.r, p.color.g, p.color.b, alpha);
        }

        // 生成拖尾
        if (!p.isTail && p.life > 0.0f) {
            Particle tail = p;
            tail.position = prevPos;
            tail.size = std::max(0.001f, p.size * 0.25f);
            float tailAlpha = 1.0f; // 固定高亮度 alpha
            tail.color = glm::vec4(1.0f, 1.0f, 1.0f, tailAlpha);
            tail.isTail = true;
            tail.life = tailLife * 0.4f;
            tail.velocity = glm::vec3(0.0f);
            tailParticles.push_back(tail);
        }

        // 检查是否爆炸
        if (p.size > (launcherSize * 0.6f) && (p.velocity.y <= 0.0f || p.life <= 0.0f)) {
            int count = 120;
            float colorType = (float)rand() / RAND_MAX;
            glm::vec3 baseColor;
            if (colorType < 0.33f) baseColor = glm::vec3(1, 0.3f + 0.2f * ((float)rand() / RAND_MAX), 0.1f + 0.1f * ((float)rand() / RAND_MAX));
            else if (colorType < 0.66f) baseColor = glm::vec3(1, 1, 0.3f + 0.2f * ((float)rand() / RAND_MAX));
            else baseColor = glm::vec3(0.2f + 0.3f * ((float)rand() / RAND_MAX), 0.4f + 0.3f * ((float)rand() / RAND_MAX), 1);
            switch (p.type) {
                case FireworkType::Sphere:
                    for (int j = 0; j < count; ++j) {
                        float phi = acos(1.0f - 2.0f * (j + 0.5f) / count);
                        float theta = 3.1415926f * (1 + sqrt(5.0f)) * j;
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(
                            sin(phi) * cos(theta),
                            -glm::abs(sin(phi) * sin(theta)),
                            cos(phi)
                        ) * (2.5f + 1.5f * ((float)rand() / RAND_MAX));
                        child.color = glm::vec4(baseColor, 1);
                        child.life = 1.2f + 0.5f * ((float)rand() / RAND_MAX);
                        child.size = childSize; // 爆炸子粒子大小（2倍）
                        child.type = p.type;
                        newExplosionParticles.push_back(child);
                    }
                    break;
                case FireworkType::Ring:
                    for (int j = 0; j < count; ++j) {
                        float angle = (float)j / count * 2.0f * 3.14159f;
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(
                            cos(angle),
                            -(0.2f + 0.8f * ((float)rand() / RAND_MAX)),
                            sin(angle)
                        ) * (3.0f + 1.0f * ((float)rand() / RAND_MAX));
                        child.color = glm::vec4(baseColor, 1);
                        child.life = 1.5f + 0.3f * ((float)rand() / RAND_MAX);
                        child.size = childSize; // 爆炸子粒子大小（2倍）
                        child.type = p.type;
                        newExplosionParticles.push_back(child);
                    }
                    break;
                case FireworkType::Heart:
                    for (int j = 0; j < count; ++j) {
                        float t = (float)j / count * 2.0f * 3.14159f;
                        float x = 16 * sin(t) * sin(t) * sin(t);
                        float y = 13 * cos(t) - 5 * cos(2*t) - 2 * cos(3*t) - cos(4*t);
                        y = -glm::abs(y);
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(x, y, 0) * 0.1f + glm::vec3(
                            ((float)rand() / RAND_MAX - 0.5f) * 0.5f,
                            ((float)rand() / RAND_MAX - 0.5f) * 0.5f,
                            ((float)rand() / RAND_MAX - 0.5f) * 0.5f
                        );
                        child.color = glm::vec4(baseColor, 1);
                        child.life = 1.8f + 0.4f * ((float)rand() / RAND_MAX);
                        child.size = childSize; // 爆炸子粒子大小（2倍）
                        child.type = p.type;
                        newExplosionParticles.push_back(child);
                    }
                    break;
                case FireworkType::Star:
                    for (int j = 0; j < count; ++j) {
                        float angle = (float)j / count * 2.0f * 3.14159f;
                        float radius = (j % 2 == 0) ? 3.5f : 2.0f;
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(
                            cos(angle) * radius,
                            -(1.0f + 0.5f * ((float)rand() / RAND_MAX)),
                            sin(angle) * radius
                        );
                        child.color = glm::vec4(baseColor, 1);
                        child.life = 1.6f + 0.4f * ((float)rand() / RAND_MAX);
                        child.size = childSize; // 爆炸子粒子大小（2倍）
                        child.type = p.type;
                        newExplosionParticles.push_back(child);
                    }
                    break;
            }
            launcherToRemove.push_back(i);
        }
    }
    // 移除爆炸的上升粒子
    for (auto it = launcherToRemove.rbegin(); it != launcherToRemove.rend(); ++it) {
        launcherParticles.erase(launcherParticles.begin() + *it);
    }
    // 添加新爆炸粒子
    explosionParticles.insert(explosionParticles.end(), newExplosionParticles.begin(), newExplosionParticles.end());

    // 为新爆炸粒子添加初始拖尾
    for (const auto& child : newExplosionParticles) {
        for (int t = 1; t <= initTailCount; ++t) {
            Particle tail = child;
            tail.position = child.position - child.velocity * (float)t * initTailStep;
            tail.size = std::max(0.004f, child.size * (1.0f - 0.2f * t));
            tail.color = glm::vec4(1.0f, 1.0f, 1.0f, child.color.a * (1.0f - 0.20f * t));
            tail.isTail = true;
            tail.life = initTailLife;
            tailParticles.push_back(tail);
        }
    }

    // 2. 更新爆炸粒子
    for (auto& p : explosionParticles) {
        glm::vec3 prevPos = p.position;
        if (p.life > 0.0f) {
            p.position += p.velocity * dt;
            p.velocity += glm::vec3(0, gravity, 0) * dt;
            p.life -= dt;
            float alpha = std::max(0.0f, p.life / alphaDecayFactor);
            p.color = glm::vec4(p.color.r, p.color.g, p.color.b, alpha);
        }
        // 生成拖尾
        if (!p.isTail && p.life > 0.0f) {
            Particle tail = p;
            tail.position = prevPos;
            tail.size = std::max(0.001f, p.size * 0.25f);
            float tailAlpha = 1.0f; // 固定高亮度 alpha
            tail.color = glm::vec4(1.0f, 1.0f, 1.0f, tailAlpha);
            tail.isTail = true;
            tail.life = tailLife * 0.4f;
            tail.velocity = glm::vec3(0.0f);
            tailParticles.push_back(tail);
        }
    }

    // 3. 更新拖尾粒子
    for (auto& p : tailParticles) {
        if (p.life > 0.0f) {
            p.position += p.velocity * dt;
            p.velocity += glm::vec3(0, gravity, 0) * dt;
            p.life -= dt;
            float alpha = std::max(0.0f, p.life / (tailLife * 0.4f)); // 慢衰减以保持亮度
            p.color = glm::vec4(p.color.r, p.color.g, p.color.b, alpha);
        }
    }

    // 清理死亡粒子
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
