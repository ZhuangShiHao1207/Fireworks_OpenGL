#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

/**
 * 点光源结构体
 * 表示场景中的一个点光源，支持永久光源和临时光源（用于烟花）
 */
struct PointLight
{
    glm::vec3 position;     // 位置
    glm::vec3 color;        // 颜色
    float intensity;        // 强度
    float lifetime;         // 剩余生命周期（秒）-1表示永久光源
    float maxLifetime;      // 最大生命周期（用于计算衰减）
    bool isPermanent;       // 是否为永久光源

    // 构造函数 - 永久光源
    PointLight(glm::vec3 pos, glm::vec3 col, float intens)
        : position(pos), color(col), intensity(intens), 
          lifetime(-1.0f), maxLifetime(-1.0f), isPermanent(true)
    {}

    // 构造函数 - 临时光源（用于烟花）
    PointLight(glm::vec3 pos, glm::vec3 col, float intens, float life)
        : position(pos), color(col), intensity(intens), 
          lifetime(life), maxLifetime(life), isPermanent(false)
    {}

    // 更新光源（返回false表示光源已死亡）
    bool Update(float deltaTime)
    {
        if (isPermanent) return true;
        
        lifetime -= deltaTime;
        
        // 根据剩余生命周期衰减强度（最后0.5秒快速衰减）
        if (lifetime < 0.5f && lifetime > 0.0f)
        {
            float fadeRatio = lifetime / 0.5f;
            intensity = intensity * fadeRatio;
        }
        
        return lifetime > 0.0f;
    }

    // 检查是否已经死亡
    bool IsDead() const
    {
        return !isPermanent && lifetime <= 0.0f;
    }
};

/**
 * 点光源管理器
 * 管理场景中的所有点光源，包括永久光源和临时光源
 * 自动处理临时光源的生命周期和衰减
 */
class PointLightManager
{
public:
    static const int MAX_LIGHTS = 16; // 最多支持16个光源（与 shader 中一致）

private:
    std::vector<PointLight> lights;

public:
    PointLightManager() {}

    // 添加永久光源（用于场景照明）
    void AddPermanentLight(glm::vec3 position, glm::vec3 color, float intensity)
    {
        if (lights.size() < MAX_LIGHTS)
        {
            lights.push_back(PointLight(position, color, intensity));
        }
    }

    // 添加临时光源（用于烟花爆炸）
    void AddTemporaryLight(glm::vec3 position, glm::vec3 color, float intensity, float lifetime)
    {
        if (lights.size() < MAX_LIGHTS)
        {
            lights.push_back(PointLight(position, color, intensity, lifetime));
        }
    }

    // 更新所有光源
    void Update(float deltaTime)
    {
        // 更新所有光源，移除已死亡的临时光源
        lights.erase(
            std::remove_if(lights.begin(), lights.end(),
                [deltaTime](PointLight& light) {
                    light.Update(deltaTime);
                    return light.IsDead();
                }),
            lights.end()
        );
    }

    // 清除所有临时光源（保留永久光源）
    void ClearTemporaryLights()
    {
        lights.erase(
            std::remove_if(lights.begin(), lights.end(),
                [](const PointLight& light) {
                    return !light.isPermanent;
                }),
            lights.end()
        );
    }

    // 清除所有光源
    void ClearAllLights()
    {
        lights.clear();
    }

    // 获取活跃光源数量
    int GetLightCount() const
    {
        return static_cast<int>(lights.size());
    }

    // 获取所有光源
    const std::vector<PointLight>& GetLights() const
    {
        return lights;
    }

    // 获取光源位置数组（用于传递给 shader）
    std::vector<glm::vec3> GetPositions() const
    {
        std::vector<glm::vec3> positions;
        for (const auto& light : lights)
            positions.push_back(light.position);
        return positions;
    }

    // 获取光源颜色数组
    std::vector<glm::vec3> GetColors() const
    {
        std::vector<glm::vec3> colors;
        for (const auto& light : lights)
            colors.push_back(light.color);
        return colors;
    }

    // 获取光源强度数组
    std::vector<float> GetIntensities() const
    {
        std::vector<float> intensities;
        for (const auto& light : lights)
            intensities.push_back(light.intensity);
        return intensities;
    }
};

#endif
