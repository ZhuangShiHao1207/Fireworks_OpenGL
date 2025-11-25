# B角色实现内容与接口介绍

## ?? 角色职责概述

**B角色**负责项目的**烟花粒子系统核心**，包括：
- 粒子系统设计与实现
- 多类型烟花爆炸（球形、环形、心形、星形等）
- 粒子生命周期、运动、重力、颜色渐变
- 多阶段爆炸、拖尾等高级特效
- 与主程序的接口对接（如摄像机矩阵、爆炸事件、渲染调用）

---

## ?? 已完成功能列表

### 1. ?? 烟花粒子系统 (`FireworkParticleSystem.h` / `FireworkParticleSystem.cpp`)

#### 功能特性
- **多类型烟花**：支持球形、环形、心形、星形等多种爆炸形状
- **粒子生命周期管理**：粒子自动消亡与复用
- **物理模拟**：支持重力、初速度、爆炸分裂
- **颜色渐变**：粒子颜色随时间插值变化
- **多阶段爆炸**：支持二次爆炸、分裂
- **拖尾效果**：可扩展支持粒子拖尾
- **接口友好**：易于与主循环、渲染、光照、音效等模块集成

#### 关键接口

```cpp
// 枚举类型：烟花类型
enum class FireworkType { Sphere, Ring, Heart, Star };

// 粒子系统核心类
class FireworkParticleSystem {
public:
    // 构造函数
    FireworkParticleSystem();

    // 发射一枚烟花
    void launch(const glm::vec3& position, const glm::vec3& velocity, FireworkType type);

    // 更新所有粒子状态
    void update(float deltaTime);

    // 渲染所有粒子
    void render();

    // 设置视图和投影矩阵（渲染用）
    void setViewProj(const glm::mat4& view, const glm::mat4& proj);

    // 可扩展接口：清空、获取粒子数等
};
```

#### 使用示例

```cpp
#include "FireworkParticleSystem.h"

// 创建粒子系统
FireworkParticleSystem fireworks;

// 在主循环中，发射烟花
if (用户按下空格) {
    fireworks.launch(glm::vec3(0,0,0), glm::vec3(0,10,0), FireworkParticleSystem::FireworkType::Sphere);
}

// 每帧更新
fireworks.update(deltaTime);

// 设置摄像机矩阵（渲染前）
fireworks.setViewProj(view, projection);

// 渲染所有粒子
fireworks.render();
```

#### 典型集成点

- **摄像机矩阵**：调用 `setViewProj` 传入主摄像机的 view/projection
- **爆炸光源**：在爆炸事件中调用 `lightManager.AddTemporaryLight`，可监听粒子系统爆炸回调
- **音效**：在爆炸事件中触发音效播放
- **渲染**：在主渲染循环中调用 `render`

---

## ??? 文件结构

```
Fireworks_OpenGL/
├── include/
│   └── FireworkParticleSystem.h   // 粒子系统头文件（接口文档与注释）
├── src/
│   └── FireworkParticleSystem.cpp // 粒子系统实现
```

---

## ?? 接口文档（头文件内已详细注释）

- 每个接口均有中英文注释，说明参数、用途、典型用法
- 支持扩展更多烟花类型和特效
- 便于与A/C/D/E角色的系统对接

---

## ?? 与其他角色的接口

- **A角色**：获取摄像机 view/projection，世界坐标
- **C角色**：粒子数据可用于后处理（如Bloom）
- **D角色**：支持UI显示当前烟花类型、数量
- **E角色**：爆炸事件可触发音效

---

## ?? 完成状态

| 功能模块         | 状态 | 完成度 |
|------------------|------|--------|
| 粒子系统         | ?   | 100%   |
| 多类型烟花       | ?   | 100%   |
| 颜色渐变         | ?   | 100%   |
| 爆炸/分裂        | ?   | 100%   |
| 拖尾/高级特效    | ?   | 可扩展 |

---

## ?? 联系方式

如有集成或接口问题，请联系B角色负责人。

---

**最后更新**: 2024年  
**版本**: 1.0  
**作者**: B角色（粒子系统）
