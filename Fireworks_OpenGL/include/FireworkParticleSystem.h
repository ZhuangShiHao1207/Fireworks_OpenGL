#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include "../include/Shader.h" // 包含Shader类

/**
 * @brief FireworkParticleSystem 烟花粒子系统核心类
 * 
 * 用于管理和渲染烟花粒子，支持多种烟花类型、颜色渐变、爆炸分裂等效果。
 * 
 * 主要接口：
 *   - launch：发射烟花
 *   - update：更新粒子状态
 *   - render：渲染粒子
 *   - setViewProj：设置视图和投影矩阵
 */
class FireworkParticleSystem {
public:
    /**
     * @brief 支持的烟花类型
     */
    enum class FireworkType { Sphere, Ring, Heart, Star };

    /**
     * @brief 构造函数，初始化粒子系统
     */
    FireworkParticleSystem();

    /**
     * @brief 析构函数，清理资源
     */
    ~FireworkParticleSystem();

    /**
     * @brief 发射一枚烟花
     * @param position 发射起点
     * @param velocity 初速度
     * @param type 烟花类型（球形、环形、心形、星形等）
     */
    void launch(const glm::vec3& position, const glm::vec3& velocity, FireworkType type);

    /**
     * @brief 更新所有粒子状态
     * @param deltaTime 时间步长（秒）
     */
    void update(float deltaTime);

    /**
     * @brief 渲染所有粒子
     */
    void render();

    /**
     * @brief 设置视图和投影矩阵（渲染用）
     * @param view 视图矩阵
     * @param proj 投影矩阵
     */
    void setViewProj(const glm::mat4& view, const glm::mat4& proj);

    // ...可扩展其他接口，如清空、获取粒子数等

private:
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec4 color;
        float life;
        float size;
        FireworkType type; // 烟花类型
        // ...?????????
    };
    std::vector<Particle> particles;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    Shader* shader; // 烟花着色器指针
    // Modern OpenGL: VAO/VBO for particle rendering
    GLuint vao = 0;
    GLuint vbo = 0;
    void initGL();
    void cleanupGL();
    bool glInited = false;
};
