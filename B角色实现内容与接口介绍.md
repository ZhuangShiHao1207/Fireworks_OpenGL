
# B角色实现内容与接口介绍

## 角色职责
B角色负责烟花粒子系统，包括：
- 粒子系统设计与实现
- 多类型烟花爆炸（球形、环形、心形、星形等）
- 粒子生命周期、运动、重力、颜色渐变
- 多阶段爆炸、拖尾等高级特效
- 与主程序接口对接（摄像机矩阵、爆炸事件、渲染调用）

## 主要接口
```cpp
enum class FireworkType { Sphere, Ring, Heart, Star };
class FireworkParticleSystem {
public:
    FireworkParticleSystem();
    void launch(const glm::vec3& position, const glm::vec3& velocity, FireworkType type);
    void update(float deltaTime);
    void render();
    void setViewProj(const glm::mat4& view, const glm::mat4& proj);
};
```

## 功能列表
- 多类型烟花：球形、环形、心形、星形等
- 粒子生命周期管理，自动消亡与复用
- 物理模拟：重力、初速度、爆炸分裂
- 颜色渐变：随时间插值变化
- 多阶段爆炸、拖尾效果
- 粒子大小随距离变化
- 烟花爆炸自动添加临时点光源
- 测试功能：runTest()方法，按键控制发射类型

## 集成说明
- 摄像机矩阵：setViewProj传入主摄像机view/projection
- 爆炸光源：爆炸事件自动添加临时点光源
- 渲染：主渲染循环调用render
- 按键控制：1-6发射不同类型烟花，0切换自动测试

## 文件结构
Fireworks_OpenGL/
├── include/FireworkParticleSystem.h
├── src/FireworkParticleSystem.cpp

## 主要修改文件
- FireworkParticleSystem.h / .cpp
- firework.vs
- InputHandler.cpp
- main.cpp
