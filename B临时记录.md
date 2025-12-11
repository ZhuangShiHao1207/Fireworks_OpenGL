当前烟花系统使用指南
1. 发射烟花（指定位置、颜色、种类）
// 在 main.cpp 或 InputHandler.cpp 中调用
fireworkSystem.launch
可用的烟花类型：

FireworkType::Sphere - 球形爆炸
FireworkType::Ring - 环形爆炸
FireworkType::MultiLayer - 多层球形（3层，不同颜色）
FireworkType::Spiral - 螺旋爆炸
FireworkType::Heart - 心形爆炸
颜色示例：

glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)  // 红色
glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)  // 绿色
glm::vec4(1.0f, 0.8f, 0.2f, 1.0f)  // 金色
glm::vec4(1.0f, 0.3f, 0.6f, 1.0f)  // 粉色
1. 添加新的烟花类型（4步）
步骤1：在头文件添加枚举
FireworkParticleSystem.h 第14-20行：

步骤2：在头文件声明生成函数
FireworkParticleSystem.h 第103-107行后添加：

步骤3：在createExplosion中添加case
FireworkParticleSystem.cpp 第281-295行，switch语句中添加：

同时在延迟爆炸的switch（第143-159行）中也添加相同的case。

步骤4：实现生成函数
在 FireworkParticleSystem.cpp 末尾添加实现：

关键参数说明：

center - 爆炸中心位置
color - 粒子颜色
count - 粒子数量（第一次爆炸240个，第二次150个）
radiusScale - 爆炸半径（第一次用默认值，第二次传5.0f）
p.velocity - 控制粒子运动方向和速度（这是定义形状的关键）
p.life - 粒子存活时间
系统自动功能：

✅ 0.1秒后自动触发第二次更大范围的爆炸
✅ 第一次爆炸自动添加光源（0.1秒，无需手动）
✅ 自动应用颜色渐变和透明度淡出
✅ 自动生成拖尾效果
✅ 自动受重力影响
3. 常用自定义形状公式
```cpp
// 圆柱形
velocity = glm::vec3(cos(angle) * r, ySpeed, sin(angle) * r);

// 圆锥形
velocity = glm::vec3(cos(angle) * r * height, height, sin(angle) * r * height);

// 螺旋上升
velocity = glm::vec3(cos(angle + height) * r, height * 2, sin(angle + height) * r);

// 十字形
if (i % 2 == 0) velocity = glm::vec3(r, 0, 0);
else velocity = glm::vec3(0, r, 0);

// 随机球形（当前Sphere实现）
velocity = glm::vec3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi)) * r;
```
这就是完整的使用和扩展流程！

