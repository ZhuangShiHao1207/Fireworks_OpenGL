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
        shader = new Shader("D:/zlearning/project/Fireworks_OpenGL/Fireworks_OpenGL/assets/shaders/firework.vs", "D:/zlearning/project/Fireworks_OpenGL/Fireworks_OpenGL/assets/shaders/firework.fs");
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    glInited = true;
}

void FireworkParticleSystem::launch(const glm::vec3& position, const glm::vec3& velocity, FireworkType type) {
    Particle p;
    p.position = position;
    p.velocity = velocity;
    p.color = glm::vec4(1, 1, 0.5, 1);
    p.life = 1.2f;
    p.size = 0.08f;
    p.type = type;
    particles.push_back(p);
}

void FireworkParticleSystem::update(float deltaTime) {
    for (auto& p : particles) {
        if (p.life > 0.0f) {
            p.position += p.velocity * deltaTime;
            p.velocity += glm::vec3(0, -9.8f, 0) * deltaTime;
            p.life -= deltaTime;
            float t = std::max(0.0f, p.life / 2.0f);
            p.color = glm::mix(glm::vec4(1,1,0.5,1), glm::vec4(0.2,0.2,1,0), 1-t);
        }
    }
    std::vector<Particle> newParticles;
    for (auto& p : particles) {
        if (p.life <= 0.0f && p.size > 0.07f) {
            int count = 60;
            switch (p.type) {
                case FireworkType::Sphere:
                    for (int i = 0; i < count; ++i) {
                        float theta = (float)i / count * 2.0f * 3.14159f;
                        float phi = ((float)rand() / RAND_MAX) * 3.14159f;
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(
                            sin(phi) * cos(theta),
                            cos(phi),
                            sin(phi) * sin(theta)
                        ) * (2.5f + 1.5f * ((float)rand() / RAND_MAX));
                        child.color = glm::vec4(1, 0.3f + 0.7f * ((float)rand() / RAND_MAX), 0.1f, 1);
                        child.life = 1.2f + 0.5f * ((float)rand() / RAND_MAX);
                        child.size = 0.04f;
                        child.type = p.type;
                        newParticles.push_back(child);
                    }
                    break;

                case FireworkType::Ring:
                    for (int i = 0; i < count; ++i) {
                        float angle = (float)i / count * 2.0f * 3.14159f;
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(
                            cos(angle),
                            0.2f + 0.8f * ((float)rand() / RAND_MAX),
                            sin(angle)
                        ) * (3.0f + 1.0f * ((float)rand() / RAND_MAX));
                        child.color = glm::vec4(0.2f + 0.8f * ((float)rand() / RAND_MAX), 1, 0.3f, 1);
                        child.life = 1.5f + 0.3f * ((float)rand() / RAND_MAX);
                        child.size = 0.04f;
                        child.type = p.type;
                        newParticles.push_back(child);
                    }
                    break;

                case FireworkType::Heart:
                    for (int i = 0; i < count; ++i) {
                        float t = (float)i / count * 2.0f * 3.14159f;
                        float x = 16 * sin(t) * sin(t) * sin(t);
                        float y = 13 * cos(t) - 5 * cos(2*t) - 2 * cos(3*t) - cos(4*t);
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(x, y, 0) * 0.1f + glm::vec3(
                            ((float)rand() / RAND_MAX - 0.5f) * 0.5f,
                            ((float)rand() / RAND_MAX - 0.5f) * 0.5f,
                            ((float)rand() / RAND_MAX - 0.5f) * 0.5f
                        );
                        child.color = glm::vec4(1, 0.2f, 0.4f + 0.6f * ((float)rand() / RAND_MAX), 1);
                        child.life = 1.8f + 0.4f * ((float)rand() / RAND_MAX);
                        child.size = 0.04f;
                        child.type = p.type;
                        newParticles.push_back(child);
                    }
                    break;

                case FireworkType::Star:
                    for (int i = 0; i < count; ++i) {
                        float angle = (float)i / count * 2.0f * 3.14159f;
                        float radius = (i % 2 == 0) ? 3.5f : 2.0f;
                        Particle child;
                        child.position = p.position;
                        child.velocity = glm::vec3(
                            cos(angle) * radius,
                            1.0f + 0.5f * ((float)rand() / RAND_MAX),
                            sin(angle) * radius
                        );
                        child.color = glm::vec4(1, 1, 0.2f + 0.8f * ((float)rand() / RAND_MAX), 1);
                        child.life = 1.6f + 0.4f * ((float)rand() / RAND_MAX);
                        child.size = 0.04f;
                        child.type = p.type;
                        newParticles.push_back(child);
                    }
                    break;
            }
            p.size = 0.0f;
        }
    }
    particles.insert(particles.end(), newParticles.begin(), newParticles.end());
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p) {
        return p.life <= 0.0f && p.size < 0.05f;
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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_POINTS, 0, (GLsizei)particles.size());

    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void FireworkParticleSystem::setViewProj(const glm::mat4& view, const glm::mat4& proj) {
    viewMatrix = view;
    projMatrix = proj;
}
