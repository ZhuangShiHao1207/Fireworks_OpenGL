#include "FireworkParticleSystem.h"
#include "miniaudio.h"
#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <algorithm>
#include <glad/glad.h>
#include <iostream>
#include <cmath>
#include <ctime>   // 用于time()函数
#include <cstdlib> // 用于rand()和srand()函数
// 随机数生成器
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> dis(0.0f, 1.0f);

extern glm::vec4 HSVtoRGB(float h, float s, float v);

FireworkParticleSystem::FireworkParticleSystem() {
    vao = 0;
    vbo = 0;
    glInited = false;
    shader = nullptr;
    lightManager = nullptr;
    // 初始化音频引擎
    ma_result result = ma_engine_init(NULL, &audioEngine);
    if (result == MA_SUCCESS) {
        audioInitialized = true;
        std::cout << "Audio engine initialized\n";
    }
    else {
        std::cerr << "Audio init failed: " << ma_result_description(result) << "\n";
    }
}

FireworkParticleSystem::~FireworkParticleSystem() {
    cleanupGL();
    if (audioInitialized) {
        ma_engine_uninit(&audioEngine);
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
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, size));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    glInited = true;
}

void FireworkParticleSystem::setLightManager(PointLightManager* manager) {
    lightManager = manager;
}

void FireworkParticleSystem::launch(const glm::vec3& position, FireworkType type, float life,
    const glm::vec4& primaryColor, const glm::vec4& secondaryColor, float size) {
    // 播放升空音效
    if (audioInitialized) {
        int index = static_cast<int>(dis(gen) * 2) % 2;
        std::string path = "assets/sounds/firework/rise/firework_rise_0" + std::to_string(index + 1) + ".wav";
        ma_engine_play_sound(&audioEngine, path.c_str(), NULL);
    }

    float randomExplosionHeight = 3.0f + dis(gen) * 4.0f;

    glm::vec3 fixedVelocity(0.0f, 12.0f, 0.0f);
    Particle p;
    float scale = 10.0;

    // 随机位置（x在-8到8之间，z在-5到5之间）
    glm::vec3 randomPos = position;
    if (position == glm::vec3(0.0f, 0.5f, 0.0f)) {
        randomPos.x = (dis(gen) * 16.0f) - 8.0f;  // -8到8
        randomPos.z = (dis(gen) * 10.0f) - 5.0f;  // -5到5
    }

    p.position = randomPos;
    p.velocity = fixedVelocity;
    p.color = primaryColor * scale;
    p.initialColor = primaryColor;
    p.secondaryColor = secondaryColor;
    p.life = life * (0.6f + dis(gen) * 0.2f);  // 随机寿命，让爆炸高度随机
    p.maxLife = p.life;
    p.size = size * 2.5f;  // 🔧 增大升空粒子大小（原本是 size，现在是 2.5 倍）
    p.type = type;
    p.isTail = false;
    p.canExplodeAgain = false;
    p.isDualColor = (secondaryColor != glm::vec4(1.0f));  // 如果是默认值，则是单色
    p.rotationAngle = 0.0f;
    p.imagePath = "";  // 默认空路径

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
			if (p.type != FireworkType::Image)
                p.color = calculateColorGradient(p);
            

            if (!p.isTail) createTail(p, prevPos);
        }
    }

    // 3. 更新延迟爆炸事件
    for (auto& delayed : delayedExplosions) {
        delayed.timer -= dt;
        if (delayed.timer <= 0.0f) {
            // 触发第二次爆炸，根据类型生成相同形状但范围更大的爆炸
            int count = 90; // 第二次爆炸粒子数
            switch (delayed.type) {
            case FireworkType::Sphere:
                generateSphereParticles(delayed.position, delayed.color, count, delayed.radius, false);
                break;
            case FireworkType::Ring:
                generateRingParticles(delayed.position, delayed.color, count, delayed.radius);
                break;
            case FireworkType::MultiLayer:
                generateMultiLayerParticles(delayed.position, delayed.color, count, delayed.radius);
                break;
            case FireworkType::Spiral:
                generateSpiralParticles(delayed.position, delayed.color, count, delayed.radius);
                break;
            case FireworkType::Heart:
                //generateHeartParticles(delayed.position, delayed.color, count, delayed.radius);
                break;
            }
            // 第二次爆炸不添加光源
        }
    }
    
    // 移除已触发的延迟爆炸
    delayedExplosions.erase(
        std::remove_if(delayedExplosions.begin(), delayedExplosions.end(),
            [](const DelayedExplosion& d) { return d.timer <= 0.0f; }),
        delayedExplosions.end()
    );

    // 4. 更新拖尾粒子
    for (auto& tail : tailParticles) {
        if (tail.life > 0.0f) {
            float t = 1.0f - (tail.life / tail.maxLife);
            tail.size = (std::max)(0.01f, tail.size * (1.0f - t * 0.05f));
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
//         float brightness = 1.0f; // 最多增强到1.15倍
//         resultColor = glm::vec4(
//             (glm::min)(p.initialColor.r * brightness, 1.0f), 
//             (glm::min)(p.initialColor.g * brightness, 1.0f), 
//             (glm::min)(p.initialColor.b * brightness, 1.0f), 
//             1.0f
//         );
        resultColor = glm::vec4(p.initialColor.r, p.initialColor.g, p.initialColor.b, 1.0f);
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
    int count = isSecondary ? 90 : 150; // 第一次爆炸粒子，第二次更多

    // 主爆炸播放音效
    if (audioInitialized && !isSecondary) {
        int index = static_cast<int>(dis(gen) * 2) % 2;
        std::string path = "assets/sounds/firework/explosion/firework_explosion_0" + std::to_string(index + 1) + ".wav";
        ma_engine_play_sound(&audioEngine, path.c_str(), NULL);
    }
    // 仅第一次爆炸添加光源，持续0.1秒
    if (lightManager && !isSecondary) {
        // 使用烟花的初始颜色（鲜艳）
        glm::vec3 lightColor(source.initialColor.r, source.initialColor.g, source.initialColor.b);
        
        // 主光源：0.1秒持续时间
        lightManager->AddTemporaryLight(source.position, lightColor, 25.0f, 0.1f);
        
        // 中心光球效果：更强的光，0.1秒
        lightManager->AddTemporaryLight(source.position, lightColor * 1.5f, 40.0f, 0.1f);
    }

    // Use initialColor instead of current color to keep explosions bright
    // 🔧 图片烟花不进行二次爆炸，避免消失时突然发亮
    if (!isSecondary && source.type != FireworkType::Image) {
        // 添加延迟0.1秒的第二次爆炸
        DelayedExplosion delayed;
        delayed.position = source.position;

        // 双色烟花使用secondaryColor，单色烟花使用initialColor
        if (source.isDualColor) {
            delayed.color = source.secondaryColor;
        }
        else {
            delayed.color = source.initialColor;
        }

        delayed.type = source.type;
        delayed.timer = 0.1f;
        delayed.radius = 5.0f; // 第二次爆炸范围更大
        delayedExplosions.push_back(delayed);
    }
    
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
    case FireworkType::Image:
        // 图片烟花使用动态路径（从粒子中获取）
        if (!source.imagePath.empty()) {
            generateImageParticles(source.position, source.imagePath, 1);
        } else {
            // 回退到默认路径
            generateImageParticles(source.position, "assets/firework_images/image.png", 1);
        }
        break;
    }
}

// 球形烟花 - 🔧 缩短生命周期
void FireworkParticleSystem::generateSphereParticles(const glm::vec3& center, const glm::vec4& color, int count, float radius, bool canExplode) {
    for (int i = 0; i < count; ++i) {
        float u = dis(gen);
        float v = dis(gen);
        float theta = u * 2.0f * 3.14159265f;
        float phi = acos(2.0f * v - 1.0f);
        float r = radius * (1.1f + 0.3f * dis(gen)); // 半径有一定随机性

        Particle p;
        p.position = center;
        p.velocity = glm::vec3(
            sin(phi) * cos(theta),
            sin(phi) * sin(theta),
            cos(phi)
        ) * r;
        p.color = color;
        p.initialColor = color;
		// 调整生命周期（0.4-0.55s）
        p.life = 0.4f + 0.15f * dis(gen);
        p.maxLife = p.life;
        p.size = childSize;
        p.type = FireworkType::Sphere;
        p.isTail = false;
        p.canExplodeAgain = canExplode;
        explosionParticles.push_back(p);
    }
}

// 环形烟花
void FireworkParticleSystem::generateRingParticles(const glm::vec3& center, const glm::vec4& color, int count, float radiusScale) {
    for (int i = 0; i < count; ++i) {
        float angle = (float)i / count * 2.0f * 3.14159265f;
        float r = radiusScale * (0.9f + 0.2f * dis(gen));

        Particle p;
        p.position = center;
        p.velocity = glm::vec3(
            cos(angle) * r,
            0.5f + dis(gen) * 0.5f, // 轻微向上
            sin(angle) * r
        );
        p.color = color;
        p.initialColor = color;
        p.life = 0.35f + 0.15f * dis(gen);  // 🔧 缩短：0.35-0.5秒（原本 0.6-0.85秒）
        p.maxLife = p.life;
        p.size = childSize;
        p.type = FireworkType::Ring;
        p.isTail = false;
        p.canExplodeAgain = false;
        explosionParticles.push_back(p);
    }
}

// 多层烟花
void FireworkParticleSystem::generateMultiLayerParticles(const glm::vec3& center, const glm::vec4& color, int count, float radiusScale) {
    int layers = 3;
    int particlesPerLayer = count / layers;
    float baseRadius = radiusScale * 0.9f;

    for (int layer = 0; layer < layers; ++layer) {
        float layerRadius = baseRadius + layer * (radiusScale * 0.3f);
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
			// 外层寿命更长 （整体寿命：）
            p.life = 0.4f + 0.15f * dis(gen) + layer * 0.1f; // 🔧 缩短：0.3-0.6秒（原本 0.5-1.1秒）
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
void FireworkParticleSystem::generateSpiralParticles(const glm::vec3& center, const glm::vec4& color, int count, float radiusScale) {
    int spirals = 3; // 3条螺旋线
    for (int i = 0; i < count; ++i) {
        int spiralIdx = i % spirals;
        float baseAngle = (float)spiralIdx / spirals * 2.0f * 3.14159265f;
        float angleOffset = (float)i / count * 4.0f * 3.14159265f; // 多圈螺旋
        float angle = baseAngle + angleOffset;
        
        float r = radiusScale * (0.4f + (float)i / count * 0.8f); // 半径逐渐增大

        Particle p;
        p.position = center;
        p.velocity = glm::vec3(
            cos(angle) * r * 0.8f,
            1.5f + dis(gen) * 0.5f,
            sin(angle) * r * 0.8f
        );
        p.color = color;
        p.initialColor = color;
        p.life = 0.45f + 0.15f * dis(gen);  // 🔧 缩短：0.45-0.6秒（原本 0.75-1.0秒）
        p.maxLife = p.life;
        p.size = childSize;
        p.type = FireworkType::Spiral;
        p.isTail = false;
        p.canExplodeAgain = false;
        p.rotationAngle = angle; // 初始旋转角度
        explosionParticles.push_back(p);
    }
}

// 心形烟花 - 🔧 缩短生命周期
void FireworkParticleSystem::generateHeartParticles(const glm::vec3& center, const glm::vec4& color, int count, float radiusScale) {
    for (int i = 0; i < count; ++i) {
        float t = (float)i / count * 2.0f * 3.14159265f;
        
        // 心形参数方程
        float x = 16.0f * pow(sin(t), 3);
        float y = 13.0f * cos(t) - 5.0f * cos(2.0f * t) - 2.0f * cos(3.0f * t) - cos(4.0f * t);
        
        // 缩放和添加随机性
        float scale = 0.15f * (radiusScale / 3.0f);
        x *= scale;
        y *= scale;

        Particle p;
        p.position = center;
        p.velocity = glm::vec3(
            x + dis(gen) * 0.3f,
            y + dis(gen) * 0.3f + 1.0f, // 向上偏移
            dis(gen) * 0.5f - 0.25f // Z方向随机
        ) * 1.5f;
        p.color = color;
        p.initialColor = color;
        p.life = 0.45f + 0.15f * dis(gen);  // 🔧 缩短：0.45-0.6秒（原本 0.75-1.0秒）
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
    static bool skipNextLaunch = false; // 跟踪是否需要跳过下一次发射

    // 每0.8秒发射一次
    if (currentTime - lastTestTime < 0.8f) return;

    lastTestTime = currentTime;

    // 如果上次是图片烟花，跳过这次发射
    if (skipNextLaunch) {
        skipNextLaunch = false;
        return;
    }

    // 生成完全随机的HSV颜色对
    auto generateRandomColorPair = []() -> std::pair<glm::vec4, glm::vec4> {
        // 随机生成主色（使用HSV模型，全范围随机）
        float hue1 = dis(gen);           // 色相：0.0-1.0 全范围
        float saturation1 = dis(gen);    // 饱和度：0.0-1.0 全范围
        float value1 = 0.5f + dis(gen) * 0.5f; // 亮度：0.5-1.0（确保颜色不太暗）

        glm::vec4 primaryColor = HSVtoRGB(hue1, saturation1, value1);

        // 生成相近的辅色（在HSV空间微调）
        // 色相偏移：-0.2到0.2之间
        float hueOffset = (dis(gen) * 0.4f) - 0.2f;
        float hue2 = fmod(hue1 + hueOffset + 1.0f, 1.0f);

        // 饱和度和亮度也随机微调（确保在0.0-1.0范围内）
        float saturation2 = glm::clamp(saturation1 + (dis(gen) * 0.3f - 0.15f), 0.0f, 1.0f);
        float value2 = glm::clamp(value1 + (dis(gen) * 0.3f - 0.15f), 0.0f, 1.0f);

        glm::vec4 secondaryColor = HSVtoRGB(hue2, saturation2, value2);

        return { primaryColor, secondaryColor };
    };

    // 随机位置（x在0到14之间，z在-9到-3之间）
    float randomX = -4.0f + dis(gen) * 14.0f;
    float randomZ = -2.5f + dis(gen) * 4.0f;
    glm::vec3 launchPos(randomX, 0.5f, randomZ);

    // 随机尺寸（0.1f到0.15f）
    float randomSize = 0.1f + dis(gen) * 0.05f;

    // 随机选择烟花类型（Image概率为15%）
    float typeRoll = dis(gen);
    FireworkType selectedType;
    
    if (typeRoll < 0.15f) {
        // 15% 概率：Image（随机选择word.png或image.png）
        selectedType = FireworkType::Image;
        std::string imagePath = (dis(gen) < 0.5f) 
            ? "assets/firework_images/word.png" 
            : "assets/firework_images/image.png";
        
        Particle launcher;
        launcher.position = launchPos;
        launcher.velocity = glm::vec3(0.0f, 12.0f, 0.0f);
        launcher.color = glm::vec4(1.0f);
        launcher.initialColor = glm::vec4(1.0f);
        launcher.secondaryColor = glm::vec4(1.0f);
        launcher.life = 1.5f * (0.4f + dis(gen) * 0.2f);
        launcher.maxLife = launcher.life;
        launcher.size = randomSize * 3.5f;
        launcher.type = selectedType;
        launcher.isTail = false;
        launcher.canExplodeAgain = false;
        launcher.isDualColor = false;
        launcher.rotationAngle = 0.0f;
        launcher.imagePath = imagePath;  // 设置图片路径
        
        // 播放升空音效
        if (audioInitialized) {
            int index = static_cast<int>(dis(gen) * 2) % 2;
            std::string path = "assets/sounds/firework/rise/firework_rise_0" + std::to_string(index + 1) + ".wav";
            ma_engine_play_sound(&audioEngine, path.c_str(), NULL);
        }
        
        launcherParticles.push_back(launcher);
        skipNextLaunch = true; // 图片烟花发射后，跳过下一次发射
        return;
    }
    else if (typeRoll < 0.15f + 0.3f) {
        // 35% 概率：双层不同色Sphere
        selectedType = FireworkType::Sphere;
        auto colors = generateRandomColorPair();
        launch(launchPos, selectedType, 1.5f, colors.first, colors.second, randomSize);
    }
    else if (typeRoll < 0.15f + 0.7f) {
        // 35% 概率：三层不同色Sphere（使用MultiLayer）
        selectedType = FireworkType::MultiLayer;
        auto colors = generateRandomColorPair();
        launch(launchPos, selectedType, 1.5f, colors.first, colors.second, randomSize);
    }
    else {
        // 15% 概率：单层Heart
        selectedType = FireworkType::Heart;
        auto colors = generateRandomColorPair();
        // 单层Heart使用相同颜色（不是双色）
        launch(launchPos, selectedType, 1.5f, colors.first, glm::vec4(1.0f), randomSize);
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

// 加载图片数据
FireworkParticleSystem::ImageData FireworkParticleSystem::loadImage(const std::string& imagePath) {
    ImageData data;
    data.width = 0;
    data.height = 0;

    int channels;
    unsigned char* imageData = stbi_load(imagePath.c_str(), &data.width, &data.height, &channels, 4); // 强制加载为RGBA
    
    if (!imageData) {
        std::cerr << "Failed to load image: " << imagePath << std::endl;
        return data;
    }

    std::cout << "Loaded image: " << imagePath << " (" << data.width << "x" << data.height << ")" << std::endl;

    // 将图片数据转换为glm::vec4数组
    data.pixels.resize(data.width * data.height);
    for (int y = 0; y < data.height; ++y) {
        for (int x = 0; x < data.width; ++x) {
            int index = (y * data.width + x) * 4;
            int pixelIndex = y * data.width + x;
            
            data.pixels[pixelIndex] = glm::vec4(
                imageData[index] / 255.0f,     // R
                imageData[index + 1] / 255.0f, // G
                imageData[index + 2] / 255.0f, // B
                imageData[index + 3] / 255.0f  // A
            );
        }
    }

    stbi_image_free(imageData);
    return data;
}

// 生成图片烟花粒子
void FireworkParticleSystem::generateImageParticles(const glm::vec3& center, const std::string& imagePath, int sampleRate) {
    // 加载图片
    ImageData image = loadImage(imagePath);
    if (image.width == 0 || image.height == 0) {
        std::cerr << "Image load failed, cannot create image firework!" << std::endl;
        return;
    }

    // 不采样，处理每个像素
    int step = 1;
    
    // 计算图片缩放比例，使其在3D空间中合适大小
    float scaleX = 4.0f / image.width;  // 图片宽度映射到4个单位
    float scaleY = 4.0f / image.height; // 图片高度映射到4个单位
    float scale = (std::min)(scaleX, scaleY); // 使用较小的缩放保持比例

    // 图片中心化
    float offsetX = (image.width * scale) / 2.0f;
    float offsetY = (image.height * scale) / 2.0f;

    int particleCount = 0;

    // 遍历图片像素，创建粒子
    for (int y = 0; y < image.height; y += step) {
        for (int x = 0; x < image.width; x += step) {
            int index = y * image.width + x;
            glm::vec4 pixelColor = image.pixels[index];

            // 跳过透明像素
            if (pixelColor.a < 0.1f) continue;

            // 计算粒子在图片中的相对位置（以图片中心为原点）
            float posX = x * scale - offsetX;
            float posY = offsetY - y * scale; // 反转Y坐标，修正上下颠倒
			//posX *= 0.2f; // 进一步缩小X轴比例，防止图片过宽
			//posY *= 0.2f; // 进一步缩小Y轴比例，防止图片过高
            Particle p;
            p.position = center; // 初始位置在爆炸中心
            
            // 速度：从中心向外扩散，保持图片形状
            // 初始速度：向图片对应位置扩散（放大效果）
            // 扩散速度基于距离中心的位置
            float expandSpeed = 0.8f; // 扩散速度系数
            p.velocity = glm::vec3(posX * expandSpeed, posY * expandSpeed, 0.0f);
            
            // 降低亮度避免bloom效果（bloom阈值为1.5）
            p.color = pixelColor * 0.11f;
            p.initialColor = pixelColor * 0.11f;
            
            p.life = 0.8f;
            p.maxLife = p.life;
            p.size = 0.015f;
            p.type = FireworkType::Image;
            p.isTail = true;  
            p.canExplodeAgain = false;
            
            explosionParticles.push_back(p);
            particleCount++;
        }
    }

    std::cout << "Created image firework with " << particleCount << " particles" << std::endl;
}