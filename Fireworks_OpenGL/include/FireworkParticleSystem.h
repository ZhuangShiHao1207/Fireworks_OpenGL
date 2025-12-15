#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include "miniaudio.h"  // 添加miniaudio音频库支持
#include "Shader.h"
#include "PointLight.h"

// FireworkParticleSystem - 烟花粒子系统
// 支持多种烟花类型、颜色渐变、二次爆炸、拖尾效果及音效
class FireworkParticleSystem {
public:
    // 支持的烟花类型（所有类型都支持二次爆炸）
    enum class FireworkType {
        Sphere,          // 球形烟花
        Ring,            // 环形烟花
        MultiLayer,      // 多层烟花
        Spiral,          // 螺旋烟花
        Heart,           // 心形烟花
        Image            // 图片烟花
    };

    FireworkParticleSystem();
    ~FireworkParticleSystem();

    // 发射一个烟花（上升弹）
    void launch(const glm::vec3& position, FireworkType type, float life, const glm::vec4& primaryColor, const glm::vec4& secondaryColor = glm::vec4(1.0f), float size = 0.05f);

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

    // 清理OpenGL资源
    void cleanupGL();

    // 拖尾参数
    float tailLife = 0.08f;        // 拖尾粒子的寿命（秒）- 减少到80%
    float tailInterval = 0.016f;    // 拖尾生成间隔（秒）
    float tailAlpha = 0.8f;         // 🔧 拖尾透明度系数（提高至0.8，原本0.5）

    // 尺寸和物理参数
    float launcherSize = 0.1f;     // 上升弹粒子大小
    float childSize = 0.35f;        // 爆炸子粒子大小
    float gravity = -5.0f;          // 重力加速度（负Y方向）
    float timeScale = 0.18f;         // 时间缩放（1.0=正常，0.5=慢动作）

private:
    struct Particle {
        glm::vec3 position;        // 位置
        glm::vec3 velocity;        // 速度
        glm::vec4 color;           // 颜色（RGBA）
        glm::vec4 initialColor;    // 初始颜色（用于颜色渐变）
        glm::vec4 secondaryColor; // 第二次爆炸颜色（仅用于dual-color烟花）
        float life;                // 剩余寿命（秒）
        float maxLife;             // 最大寿命（用于计算生命周期比例）
        float size;                // 规格化尺寸（0..1），在顶点着色器中转换为像素大小
        FireworkType type;         // 烟花类型
        bool isDualColor = false; // 是否为双色烟花
        bool isTail = false;       // 是否为拖尾粒子，防止递归产生尾迹
        bool canExplodeAgain = false; // 是否可以二次爆炸
        float rotationAngle = 0.0f;   // 旋转角度（用于螺旋烟花）
        float tailTimer = 0.0f;       // 拖尾生成计时器
        float explodeAtHeight = 0.0f; // 随机爆炸高度
        std::string imagePath = "";  // 图片烟花的路径（仅对Image类型有效）
    };

    // 图片数据结构
    struct ImageData {
        std::vector<glm::vec4> pixels; // 像素颜色数组
        int width;
        int height;
    };

    // 延迟爆炸事件结构
    struct DelayedExplosion {
        glm::vec3 position;        // 爆炸位置
        glm::vec4 color;           // 爆炸颜色
        FireworkType type;         // 烟花类型
        float timer;               // 倒计时（秒）
        float radius;              // 爆炸半径
    };

    std::vector<Particle> launcherParticles;   // 上升粒子
    std::vector<Particle> explosionParticles;  // 爆炸粒子
    std::vector<Particle> tailParticles;       // 拖尾粒子
    std::vector<DelayedExplosion> delayedExplosions; // 延迟二次爆炸事件
    std::vector<Particle> particles;           // 渲染时合并所有粒子的容器

    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    Shader* shader = nullptr;
    PointLightManager* lightManager = nullptr;

    // OpenGL 对象
    GLuint vao = 0;
    GLuint vbo = 0;
    void initGL();
    bool glInited = false;

    // 音频引擎
    ma_engine audioEngine;          // miniaudio引擎实例
    bool audioInitialized = false;  // 音频初始化状态标志

    // 辅助方法
    void createExplosion(const Particle& source, bool isSecondary = false);
    glm::vec4 calculateColorGradient(const Particle& p) const;
    void generateSphereParticles(const glm::vec3& center, const glm::vec4& color, int count, float radius = 4.0f, bool canExplode = false);
    void generateRingParticles(const glm::vec3& center, const glm::vec4& color, int count, float radiusScale = 3.5f);
    void generateMultiLayerParticles(const glm::vec3& center, const glm::vec4& color, int count, float radiusScale = 3.0f);
    void generateSpiralParticles(const glm::vec3& center, const glm::vec4& color, int count, float radiusScale = 4.0f);
    void generateHeartParticles(const glm::vec3& center, const glm::vec4& color, int count, float radiusScale = 3.0f);
    void generateImageParticles(const glm::vec3& center, const std::string& imagePath, int sampleRate = 2);
    
    // 图片加载辅助方法
    ImageData loadImage(const std::string& imagePath);
};