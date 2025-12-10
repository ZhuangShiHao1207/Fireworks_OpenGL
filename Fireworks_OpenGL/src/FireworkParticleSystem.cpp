#include "FireworkParticleSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <algorithm>
#include <glad/glad.h>
#include <iostream>
#include <cmath>

// 随机数生成器
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> dis(0.0f, 1.0f);

FireworkParticleSystem::FireworkParticleSystem() {
    vao = 0;
    vbo = 0;
    glInited = false;
    shader = nullptr;
    lightManager = nullptr;
}

FireworkParticleSystem::~FireworkParticleSystem() {
    // 清理OpenGL资源
    cleanupGL();
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
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, size));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    glInited = true;
}

void FireworkParticleSystem::setLightManager(PointLightManager* manager) {
    lightManager = manager;
}

void FireworkParticleSystem::launch(const glm::vec3& position, FireworkType type, float life, const glm::vec4& color, float size) {
    glm::vec3 fixedVelocity(0.0f, 12.0f, 0.0f);

    Particle p;
    float scale = 10.0;

    p.position = position;
    p.velocity = fixedVelocity;
    p.color = color * scale;
    p.initialColor = color;
    p.life = life;
    p.maxLife = life;
    p.size = size;
    p.type = type;
    p.isTail = false;
    p.canExplodeAgain = (type == FireworkType::DoubleExplosion);
    p.rotationAngle = 0.0f;
    launcherParticles.push_back(p);
}


void FireworkParticleSystem::update(float deltaTime) {
    float dt = deltaTime * timeScale;
    
    // Lambda: 创建拖尾粒子（合并重复逻辑）
    auto createTail = [&](const Particle& parent, const glm::vec3& position) {
        Particle tail = parent;
        tail.position = position;
        tail.isTail = true;
        tail.life = tailLife;
        tail.maxLife = tailLife;
        tail.velocity = glm::vec3(0.0f);
        tail.color.a *= tailAlpha;
        tail.initialColor.a *= tailAlpha;
        tailParticles.push_back(tail);
    };

    // 1. 更新上升粒子
    std::vector<Particle> toExplode;
    for (auto& p : launcherParticles) {
        if (p.life > 0.0f) {
            glm::vec3 prevPos = p.position;
            p.position += p.velocity * dt;
            p.velocity += glm::vec3(0, gravity, 0) * dt;
            p.life -= dt;
            p.color = calculateColorGradient(p);
            
            if (!p.isTail) createTail(p, prevPos);
        }

        if (p.velocity.y <= 0.0f || p.life <= 0.0f) {
            toExplode.push_back(p);
        }
    }

    for (const auto& p : toExplode) {
        createExplosion(p, false);
    }

    // 2. 更新爆炸粒子
    std::vector<Particle> toExplodeAgain;
    for (auto& p : explosionParticles) {
        if (p.life > 0.0f) {
            glm::vec3 prevPos = p.position;
            
            // 螺旋烟花旋转
            if (p.type == FireworkType::Spiral) {
                p.rotationAngle += dt * 3.0f;
                float radius = glm::length(glm::vec2(p.velocity.x, p.velocity.z));
                p.velocity.x = radius * cos(p.rotationAngle);
                p.velocity.z = radius * sin(p.rotationAngle);
            }
            
            p.position += p.velocity * dt;
            p.velocity += glm::vec3(0, gravity, 0) * dt;
            p.life -= dt;
            p.color = calculateColorGradient(p);
            
            createTail(p, prevPos);

            // 二次爆炸检测
            if (p.canExplodeAgain && p.life < p.maxLife * 0.5f && p.life > p.maxLife * 0.4f) {
                toExplodeAgain.push_back(p);
                p.canExplodeAgain = false;
            }
        }
    }

    for (const auto& p : toExplodeAgain) {
        createExplosion(p, true);
    }

    // 3. 更新拖尾粒子
    for (auto& tail : tailParticles) {
        if (tail.life > 0.0f) {
            float t = 1.0f - (tail.life / tail.maxLife);
            tail.size = std::max(0.01f, tail.size * (1.0f - t * 0.1f));
            tail.life -= dt;
            tail.color = calculateColorGradient(tail);
        }
    }

    // 移除死亡粒子（合并条件）
    auto isDead = [](const Particle& p) { return p.life <= 0.0f || p.position.y < 0.0f; };
    launcherParticles.erase(std::remove_if(launcherParticles.begin(), launcherParticles.end(), isDead), launcherParticles.end());
    explosionParticles.erase(std::remove_if(explosionParticles.begin(), explosionParticles.end(), isDead), explosionParticles.end());
    tailParticles.erase(std::remove_if(tailParticles.begin(), tailParticles.end(), isDead), tailParticles.end());
}

void FireworkParticleSystem::render() {
    if (!glInited) initGL();

    // 合并所有粒子到 particles 用于渲染
    particles.clear();
    particles.insert(particles.end(), launcherParticles.begin(), launcherParticles.end());
    particles.insert(particles.end(), explosionParticles.begin(), explosionParticles.end());
    particles.insert(particles.end(), tailParticles.begin(), tailParticles.end());

    if (particles.empty() || !shader) return;

    shader->use();
    shader->setMat4("view", viewMatrix);
    shader->setMat4("projection", projMatrix);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // 标准alpha混合，避免叠加变色
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDepthMask(GL_FALSE); // 关闭深度写入，但保留深度测试

    glDrawArrays(GL_POINTS, 0, (GLsizei)particles.size());

    glDepthMask(GL_TRUE);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void FireworkParticleSystem::setViewProj(const glm::mat4& view, const glm::mat4& proj) {
    viewMatrix = view;
    projMatrix = proj;
}

// 颜色渐变计算：初始亮 → 中段彩色 → 消失
glm::vec4 FireworkParticleSystem::calculateColorGradient(const Particle& p) const {
    float lifeRatio = p.life / p.maxLife;
    glm::vec4 resultColor = p.initialColor;

    if (lifeRatio > 0.95f) {
        // 初始阶段：稍微明亮（缩短到只有最开始5%的时间）
        float brightness = 1.0f; // 最多增强到1.15倍
        resultColor = glm::vec4(
            glm::min(p.initialColor.r * brightness, 1.0f), 
            glm::min(p.initialColor.g * brightness, 1.0f), 
            glm::min(p.initialColor.b * brightness, 1.0f), 
            1.0f
        );
    }
    else if (lifeRatio > 0.15f) {
        // 中段：保持原色明亮（延长保持原色的时间）
        resultColor = glm::vec4(p.initialColor.r, p.initialColor.g, p.initialColor.b, 1.0f);
    }
    else {
        // 末段：快速淡出（只在最后15%生命值时）
        float fadeRatio = lifeRatio / 0.15f;
        resultColor = glm::vec4(p.initialColor.r, p.initialColor.g, 
                                p.initialColor.b, fadeRatio);
    }

    return resultColor;
}

// 创建爆炸粒子
void FireworkParticleSystem::createExplosion(const Particle& source, bool isSecondary) {
    // Particles are self-illuminating, no scene lights needed
    int count = isSecondary ? 60 : 120; // 二次爆炸粒子数较少

    // Use initialColor instead of current color to keep explosions bright
    switch (source.type) {
    case FireworkType::Sphere:
        generateSphereParticles(source.position, source.initialColor, count);
        break;
    case FireworkType::Ring:
        generateRingParticles(source.position, source.initialColor, count);
        break;
    case FireworkType::MultiLayer:
        generateMultiLayerParticles(source.position, source.initialColor, count);
        break;
    case FireworkType::Spiral:
        generateSpiralParticles(source.position, source.initialColor, count);
        break;
    case FireworkType::Heart:
        generateHeartParticles(source.position, source.initialColor, count);
        break;
    case FireworkType::DoubleExplosion:
        generateSphereParticles(source.position, source.initialColor, count);
        break;
    }
}

// 球形烟花
void FireworkParticleSystem::generateSphereParticles(const glm::vec3& center, const glm::vec4& color, int count) {
    for (int i = 0; i < count; ++i) {
        float u = dis(gen);
        float v = dis(gen);
        float theta = u * 2.0f * 3.14159265f;
        float phi = acos(2.0f * v - 1.0f);
        float r = 2.5f + 1.5f * dis(gen);

        Particle p;
        p.position = center;
        p.velocity = glm::vec3(
            sin(phi) * cos(theta),
            sin(phi) * sin(theta),
            cos(phi)
        ) * r;
        p.color = color;
        p.initialColor = color;
        p.life = 1.2f + 0.5f * dis(gen);
        p.maxLife = p.life;
        p.size = childSize;
        p.type = FireworkType::Sphere;
        p.isTail = false;
        p.canExplodeAgain = false;
        explosionParticles.push_back(p);
    }
}

// 环形烟花
void FireworkParticleSystem::generateRingParticles(const glm::vec3& center, const glm::vec4& color, int count) {
    for (int i = 0; i < count; ++i) {
        float angle = (float)i / count * 2.0f * 3.14159265f;
        float r = 3.0f + 0.5f * dis(gen);

        Particle p;
        p.position = center;
        p.velocity = glm::vec3(
            cos(angle) * r,
            0.5f + dis(gen) * 0.5f, // 轻微向上
            sin(angle) * r
        );
        p.color = color;
        p.initialColor = color;
        p.life = 1.2f + 0.5f * dis(gen);
        p.maxLife = p.life;
        p.size = childSize;
        p.type = FireworkType::Ring;
        p.isTail = false;
        p.canExplodeAgain = false;
        explosionParticles.push_back(p);
    }
}

// 多层烟花
void FireworkParticleSystem::generateMultiLayerParticles(const glm::vec3& center, const glm::vec4& color, int count) {
    int layers = 3;
    int particlesPerLayer = count / layers;

    for (int layer = 0; layer < layers; ++layer) {
        float layerRadius = 2.0f + layer * 1.0f;
        glm::vec4 layerColor = color;
        
        // 每层不同颜色变化
        if (layer == 1) {
            layerColor = glm::vec4(color.r * 0.8f, color.g * 1.2f, color.b * 0.9f, color.a);
        }
        else if (layer == 2) {
            layerColor = glm::vec4(color.r * 1.2f, color.g * 0.9f, color.b * 1.1f, color.a);
        }

        for (int i = 0; i < particlesPerLayer; ++i) {
            float u = dis(gen);
            float v = dis(gen);
            float theta = u * 2.0f * 3.14159265f;
            float phi = acos(2.0f * v - 1.0f);

            Particle p;
            p.position = center;
            p.velocity = glm::vec3(
                sin(phi) * cos(theta),
                sin(phi) * sin(theta),
                cos(phi)
            ) * layerRadius;
            p.color = layerColor;
            p.initialColor = layerColor;
            p.life = 1.0f + 0.3f * dis(gen) + layer * 0.3f; // 外层寿命更长
            p.maxLife = p.life;
            p.size = childSize * (1.0f + layer * 0.2f); // 外层更大
            p.type = FireworkType::MultiLayer;
            p.isTail = false;
            p.canExplodeAgain = false;
            explosionParticles.push_back(p);
        }
    }
}

// 螺旋烟花
void FireworkParticleSystem::generateSpiralParticles(const glm::vec3& center, const glm::vec4& color, int count) {
    int spirals = 3; // 3条螺旋线
    for (int i = 0; i < count; ++i) {
        int spiralIdx = i % spirals;
        float baseAngle = (float)spiralIdx / spirals * 2.0f * 3.14159265f;
        float angleOffset = (float)i / count * 4.0f * 3.14159265f; // 多圈螺旋
        float angle = baseAngle + angleOffset;
        
        float r = 2.0f + (float)i / count * 2.0f; // 半径逐渐增大

        Particle p;
        p.position = center;
        p.velocity = glm::vec3(
            cos(angle) * r * 0.8f,
            1.5f + dis(gen) * 0.5f,
            sin(angle) * r * 0.8f
        );
        p.color = color;
        p.initialColor = color;
        p.life = 1.5f + 0.5f * dis(gen);
        p.maxLife = p.life;
        p.size = childSize;
        p.type = FireworkType::Spiral;
        p.isTail = false;
        p.canExplodeAgain = false;
        p.rotationAngle = angle; // 初始旋转角度
        explosionParticles.push_back(p);
    }
}

// 心形烟花
void FireworkParticleSystem::generateHeartParticles(const glm::vec3& center, const glm::vec4& color, int count) {
    for (int i = 0; i < count; ++i) {
        float t = (float)i / count * 2.0f * 3.14159265f;
        
        // 心形参数方程
        float x = 16.0f * pow(sin(t), 3);
        float y = 13.0f * cos(t) - 5.0f * cos(2.0f * t) - 2.0f * cos(3.0f * t) - cos(4.0f * t);
        
        // 缩放和添加随机性
        float scale = 0.15f;
        x *= scale;
        y *= scale;

        Particle p;
        p.position = center;
        p.velocity = glm::vec3(
            x + dis(gen) * 0.3f,
            y + dis(gen) * 0.3f + 1.0f, // 向上偏移
            dis(gen) * 0.5f - 0.25f // Z方向随机
        );
        p.color = color;
        p.initialColor = color;
        p.life = 1.5f + 0.5f * dis(gen);
        p.maxLife = p.life;
        p.size = childSize;
        p.type = FireworkType::Heart;
        p.isTail = false;
        p.canExplodeAgain = false;
        explosionParticles.push_back(p);
    }
}

// 测试方法：依次发射各种类型烟花
void FireworkParticleSystem::runTest(float currentTime) {
    static float lastTestTime = 0.0f;
    static int testPhase = 0;
    
    if (currentTime - lastTestTime < 3.0f) return; // 每3秒发射一次
    
    lastTestTime = currentTime;
    
    // 定义测试配置
    struct TestConfig {
        FireworkType type;
        glm::vec3 position;
        glm::vec4 color;
        float size;
        const char* name;
    };
    
    TestConfig tests[] = {
        { FireworkType::Sphere, glm::vec3(-6.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.2f, 0.2f, 1.0f), 0.03f, "Sphere (Red)" },
        { FireworkType::Ring, glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec4(0.2f, 1.0f, 0.2f, 1.0f), 0.03f, "Ring (Green)" },
        { FireworkType::MultiLayer, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.2f, 0.5f, 1.0f, 1.0f), 0.025f, "MultiLayer (Blue)" },
        { FireworkType::Spiral, glm::vec3(3.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.8f, 0.2f, 1.0f), 0.025f, "Spiral (Gold)" },
        { FireworkType::Heart, glm::vec3(6.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.3f, 0.6f, 1.0f), 0.03f, "Heart (Pink)" },
        { FireworkType::DoubleExplosion, glm::vec3(-4.5f, 0.0f, -3.0f), glm::vec4(0.8f, 0.3f, 1.0f, 1.0f), 0.035f, "DoubleExplosion (Purple)" },
    };
    
    int numTests = sizeof(tests) / sizeof(tests[0]);
    
    if (testPhase < numTests) {
        TestConfig& config = tests[testPhase];
        launch(config.position, config.type, 1.5f, config.color, config.size);
        std::cout << "[Test] Launch " << config.name << " at (" 
                  << config.position.x << ", " << config.position.y << ", " << config.position.z << ")" << std::endl;
        testPhase++;
    } else {
        testPhase = 0; // Loop test
    }
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