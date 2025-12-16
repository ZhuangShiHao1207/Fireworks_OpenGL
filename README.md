# 🧨 烟花粒子系统

## ✨ 一、项目简介

本项目基于 **OpenGL 3.3 + GLFW/GLAD** 搭建，实现了一个功能完整、视觉精美的 **3D 烟花粒子系统**。项目集成了粒子物理、动态光照、后处理渲染、3D场景建模、音效系统、交互控制等多个模块，提供了沉浸式的视觉体验。

### 核心特性

#### 1. 多样化烟花粒子系统

- **6种烟花类型**：球形、环形、心形、螺旋、多层、图片烟花
- **双色二次爆炸**：主色+辅色的二次绽放效果
- **物理模拟**：重力加速度、空气阻力、初速度控制
- **粒子拖尾**：上升阶段的动态拖尾特效
- **颜色渐变**：从亮色到暗色的自然过渡

#### 2. 完整的3D场景渲染

- **立方体贴图天空盒**：6面夜空纹理，营造沉浸感
- **Blinn-Phong光照地面**：支持纹理贴图、镜面反射、双重雾效（距离雾+高度雾）
- **Assimp模型加载**：书本3D模型，支持多网格、多纹理
- **点光源系统**：最多16个动态光源（4个永久场景光+12个临时烟花光）

#### 3. 摄像机与演示系统

- **自由视角模式**：WASD移动 + 鼠标控制视角
- **轨道环绕模式**（按键R）：自动围绕场景旋转
- **电影运镜模式**（按键9）：11秒三阶段自动镜头动画
  - 高空俯瞰下降 → 环绕飞行+晃动 → 降落至观赏点
- **淡入淡出后处理**：阶段切换时的平滑过渡效果
- **自动演示模式**：按键0启动自动烟花序列，按键9+0实现完整演示 

#### 4. 后处理与资源管理

- **FBO离屏渲染**：场景先渲染到帧缓冲，再应用后处理
- **音效系统**：miniaudio引擎，烟花上升/爆炸音效
- **资源统一管理**：OpenGL对象的规范化初始化与清理

#### 5. 交互式UI系统

- **FreeType文本渲染**：支持中英文显示
- **实时信息**：FPS、烟花计数、光源数量、操作提示
- **艺术字动画**：F1-F6触发的动态文字特效



## 🧩 二、组内分工

| 团队成员 | 职责               | 具体                                                         |
| -------- | ------------------ | ------------------------------------------------------------ |
| 庄仕豪   | 主架构与展示       | 1、实现摄像机、天空盒、地面、模型、坐标系统。设计整体项目并搭建结构<br />2、实现摄像机轨道、画面的淡入淡出后处理、烟花展示序列与整体参数调试<br />3、编写文档和制作ppt |
| 周宏杰   | 粒子模型与烟花逻辑 | 1、实现烟花粒子的基础模型和烟花发射，爆炸，更新与二次爆炸。<br />2、自定义烟花类型和拖尾函数；实现图片类型烟花相关功能 |
| 郑浩     | 后处理与资源管理   | 1、后处理渲染流程：离屏渲染、亮部提取、高斯模糊等<br />2、资源管理：对 OpenGL 资源进行统一初始化、使用与释放管理，保证系统稳定性。 |
| 钟晨晖   | UI界面与艺术字     | 1、 响应式UI布局、鼠标射线检测、状态跟踪、文本渲染<br />2、艺术字动画、颜色状态显示、数据监控、操作提示、中文支持 |
| 王天赐   | 音效与随机化       | 烟花音效与随机烟花颜色、位置、高度                           |

------

## 🎥 三、展示视频

视频链接：[中大烟花粒子系统——计图期末作业_ 哔哩哔哩_ bilibili](https://www.bilibili.com/video/BV1dmqhB3EUq/?vd_source=11f2137d6276ac03fd493958370ff17e)



## 🎯 四、实现方法

### 1. 渲染管线架构

#### 1.1 OpenGL渲染流程

项目采用**双缓冲+FBO离屏渲染**架构，完整渲染管线如下：

```
主渲染循环（main.cpp）
├─ postProcessor->Bind()  // 绑定FBO，所有场景渲染到离屏纹理
├─ 场景渲染阶段
│  ├─ 清空颜色+深度缓冲
│  ├─ 计算MVP矩阵（projection, view, model）
│  ├─ 绘制地面（Blinn-Phong光照+雾效）
│  ├─ 绘制模型（Assimp加载的书本）
│  ├─ 绘制天空盒（深度测试优化）
│  └─ 绘制烟花粒子（半透明叠加）
├─ postProcessor->Unbind()  // 恢复默认帧缓冲
├─ 后处理阶段
│  ├─ 获取电影模式淡入淡出alpha
│  ├─ 应用后处理shader（全屏四边形）
│  └─ 渲染到屏幕
├─ UI渲染（FreeType文本）
└─ glfwSwapBuffers()  // 交换前后缓冲
```

**关键技术点**：

- **FBO离屏渲染**：场景先渲染到纹理，便于后处理（如淡入淡出、泛光等）
- **深度测试优化**：天空盒使用`GL_LEQUAL`确保在最远处，烟花粒子关闭深度写入但保留深度测试
- **渲染顺序**：不透明物体（地面/模型）→ 天空盒 → 半透明粒子（由远到近排序）

#### 1.2 坐标系统与MVP变换

**OpenGL右手坐标系**：

- X轴：右为正
- Y轴：上为正
- Z轴：屏幕外为正（观察者朝向-Z）

**MVP变换管线**：

```cpp
// 1. 模型矩阵（局部空间 → 世界空间）
glm::mat4 model = glm::translate(...) * glm::rotate(...) * glm::scale(...);

// 2. 视图矩阵（世界空间 → 观察空间）
glm::mat4 view = camera.GetViewMatrix();  // glm::lookAt

// 3. 投影矩阵（观察空间 → 裁剪空间）
glm::mat4 projection = glm::perspective(
    glm::radians(camera.Zoom),  // FOV = 45度
    (float)SCR_WIDTH / SCR_HEIGHT,
    0.1f,   // 近平面
    200.0f  // 远平面
);

// 顶点着色器应用
gl_Position = projection * view * model * vec4(aPos, 1.0);
```

---

### 2. 摄像机系统

#### 2.1 欧拉角实现

使用**Yaw（偏航角）和Pitch（俯仰角）**控制摄像机朝向：

```cpp
glm::vec3 front;
front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
front.y = sin(glm::radians(Pitch));
front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
Front = glm::normalize(front);
```

**万向锁解决**：限制Pitch在[-89°, 89°]，避免垂直朝向导致的自由度丢失。

#### 2.2 三种摄像机模式

**自由模式**：

- 键盘：`Position += Front * velocity`（WASD）
- 鼠标：`Yaw += xoffset`, `Pitch += yoffset`

**轨道模式**（按键R）：

```cpp
float angleRad = glm::radians(OrbitAngle);
Position.x = OrbitCenter.x + OrbitRadius * cos(angleRad);
Position.z = OrbitCenter.z + OrbitRadius * sin(angleRad);
Front = normalize(OrbitCenter - Position);  // 始终注视中心
```

**电影模式**（按键9）：

- **阶段一（0-3.5s）**：高空俯瞰，`Position.y`从7降至4，`Position.z`从27降至17
- **阶段二（3.5-8.5s）**：环绕轨道22单位半径，叠加sin/cos晃动（幅度0.12）
- **阶段三（8.5-11s）**：使用`smoothstep`平滑降落，Pitch从-8°升至15°（仰视）

**淡入淡出**：在阶段切换时（3.0-4.0s, 8.0-9.0s）返回alpha值，应用到后处理shader。

---

### 3. 场景渲染模块

#### 3.1 天空盒系统

**立方体贴图原理**：

- 使用6张纹理（px/nx/py/ny/pz/nz）构成cubemap
- 顶点着色器：`TexCoords = aPos`（顶点位置直接作为采样方向）
- 深度优化：`gl_Position = (projection * view * vec4(aPos, 1.0)).xyww`
  - 将z分量替换为w，确保深度值恒为1.0（最远处）
  - 配合`glDepthFunc(GL_LEQUAL)`通过深度测试

**亮度调整**：通过在片段着色器乘以系数（0.3）降低夜空亮度，匹配地面雾效颜色。

#### 3.2 地面系统（Blinn-Phong + 双重雾效）

**Blinn-Phong光照模型**：

```glsl
// 环境光
vec3 ambient = 0.1 * lightColor;

// 漫反射
float diff = max(dot(normal, lightDir), 0.0);
vec3 diffuse = diff * lightColor;

// Blinn-Phong镜面反射
vec3 halfwayDir = normalize(lightDir + viewDir);
float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
vec3 specular = specularStrength * spec * lightColor;
```

**双重雾效**：

```glsl
// 距离雾（指数衰减）
float distance = length(FragPos - viewPos);
float fogFactor1 = exp(-fogDensity * max(0.0, distance - fogStart));

// 高度雾（Y轴衰减）
float heightFactor = max(0.0, FragPos.y - 0.0) / 10.0;
float fogFactor2 = 1.0 - clamp(heightFactor, 0.0, 1.0);

// 组合雾效
float finalFog = fogFactor1 * (1.0 - fogFactor2 * 0.3);
color = mix(fogColor, color, finalFog);
```

**纹理UV平铺**：`TexCoords = aTexCoords * textureRepeat`（repeat=20），实现大尺寸地面的纹理重复。

#### 3.3 模型加载系统（Assimp）

**递归场景图遍历**：

```cpp
void Model::processNode(aiNode* node, const aiScene* scene) {
    // 处理当前节点的所有网格
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // 递归处理子节点
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}
```

**纹理路径容错**：

- 提取裸文件名（去除路径）
- 在`模型目录/textures/`、`模型目录/`、`模型父目录/textures/`等多路径搜索
- 找到第一个存在的文件即加载，提升兼容性

**Mesh数据结构**：

```cpp
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Mesh {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO, VBO, EBO;
    
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        // 上传vertices到VBO，indices到EBO
        // 配置顶点属性指针
    }
};
```

---

### 4. 烟花粒子系统

#### 4.1 粒子数据结构

```cpp
struct Particle {
    glm::vec3 position, velocity;
    glm::vec4 color, initialColor, secondaryColor;
    float life, maxLife, size;
    FireworkType type;
    bool isTail, canExplodeAgain, isDualColor;
    float rotationAngle;  // 螺旋烟花用
    std::string imagePath;  // 图片烟花用
};
```

#### 4.2 物理模拟

**重力与空气阻力**：

```cpp
void update(float deltaTime) {
    p.velocity += glm::vec3(0, gravity, 0) * dt;  // gravity = -9.8
    p.velocity *= 0.993f;  // 空气阻力
    p.position += p.velocity * dt;
    p.life -= dt;
}
```

**拖尾粒子**：

```cpp
Particle tail = parent;
tail.position = prevPos;  // 上一帧位置
tail.isTail = true;
tail.life = 0.3f;  // 短生命周期
tail.velocity = glm::vec3(0.0f);  // 静止
tail.color.a *= 0.5f;  // 半透明
```

#### 4.3 六种烟花类型实现

**球形烟花（均匀分布）**：

```cpp
float theta = u * 2π;  // 方位角
float phi = acos(2v - 1);  // 极角（均匀分布）
velocity = vec3(sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi)) * radius;
```

**环形烟花（水平圆环）**：

```cpp
float angle = (i / count) * 2π;
velocity = vec3(cos(angle)*r, 0.5+random, sin(angle)*r);
```

**心形烟花（参数方程）**：

```cpp
float x = 16 * pow(sin(t), 3);
float y = 13*cos(t) - 5*cos(2t) - 2*cos(3t) - cos(4t);
velocity = vec3(x, y, random_z) * scale;
```

**螺旋烟花（旋转扩散）**：

```cpp
// 更新时旋转速度方向
p.rotationAngle += dt * 3.0f;
float radius = length(vec2(p.velocity.x, p.velocity.z));
p.velocity.x = radius * cos(p.rotationAngle);
p.velocity.z = radius * sin(p.rotationAngle);
```

**多层烟花（3层不同半径）**：

```cpp
for (int layer = 0; layer < 3; layer++) {
    float layerRadius = baseRadius + layer * 0.3;
    // 每层不同颜色变化
    if (layer == 1) color = color * vec3(0.8, 1.2, 0.9);
    // 外层寿命更长
    life = baseLife + layer * 0.1f;
}
```

**图片烟花（像素采样）**：

```cpp
ImageData img = loadImage(path);  // stb_image加载
for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
        if (alpha < 0.1f) continue;  // 跳过透明像素
        float posX = x * scale - offsetX;  // 中心化
        float posY = offsetY - y * scale;  // 翻转Y
        velocity = vec3(posX, posY, 0) * expandSpeed;
        color = pixelColor * 0.2f;  // 降低亮度避免bloom
    }
}
```

#### 4.4 双色二次爆炸

**延迟爆炸机制**：

```cpp
struct DelayedExplosion {
    glm::vec3 position;
    glm::vec4 color;  // 使用secondaryColor
    FireworkType type;
    float timer;  // 0.1秒后触发
    float radius;  // 第二次爆炸范围更大
};

// 第一次爆炸时添加延迟事件
if (!isSecondary && type != Image) {
    DelayedExplosion delayed;
    delayed.color = isDualColor ? secondaryColor : initialColor;
    delayed.timer = 0.1f;
    delayed.radius = 5.0f;
    delayedExplosions.push_back(delayed);
}
```

#### 4.5 颜色渐变算法

```cpp
glm::vec4 calculateColorGradient(const Particle& p) {
    float lifeRatio = p.life / p.maxLife;
    if (lifeRatio > 0.95f) {
        // 初始5%：保持原色
        return vec4(initialColor.rgb, 1.0);
    } else if (lifeRatio > 0.15f) {
        // 中段80%：保持明亮
        return vec4(initialColor.rgb, 1.0);
    } else {
        // 末段15%：快速淡出
        float fade = lifeRatio / 0.15f;
        return vec4(initialColor.rgb * fade, 1.0);
    }
}
```

---

### 5. 动态光照系统

#### 5.1 点光源管理器

```cpp
class PointLightManager {
    struct PointLight {
        glm::vec3 position, color;
        float intensity, lifetime;
        bool isPermanent;
    };
    vector<PointLight> lights;  // 最多16个
    
    void AddPermanentLight(...);  // 场景固定光源
    void AddTemporaryLight(...);  // 烟花临时光源
    void Update(float dt) {
        // 移除过期的临时光源
        lights.erase(remove_if(lights.begin(), lights.end(),
            [](auto& l) { return !l.isPermanent && l.lifetime <= 0; }));
    }
};
```

#### 5.2 Shader中的多光源计算

```glsl
uniform vec3 lightPositions[16];
uniform vec3 lightColors[16];
uniform float lightIntensities[16];
uniform int numLights;

for (int i = 0; i < numLights; i++) {
    vec3 lightDir = normalize(lightPositions[i] - FragPos);
    float distance = length(lightPositions[i] - FragPos);
    float attenuation = lightIntensities[i] / (1.0 + 0.09*distance + 0.032*distance*distance);
    
    // Blinn-Phong计算
    vec3 result = (ambient + diffuse + specular) * attenuation * lightColors[i];
    finalColor += result;
}
```

#### 5.3 烟花爆炸光源联动

```cpp
void createExplosion(...) {
    if (lightManager && !isSecondary) {
        vec3 lightColor(initialColor.rgb);
        // 主光源：0.1秒
        lightManager->AddTemporaryLight(position, lightColor, 25.0f, 0.1f);
        // 中心光球：更强
        lightManager->AddTemporaryLight(position, lightColor*1.5f, 40.0f, 0.1f);
    }
}
```

---

### 6. 后处理系统

#### 6.1 FBO离屏渲染

```cpp
// 初始化
glGenFramebuffers(1, &FBO);
glBindFramebuffer(GL_FRAMEBUFFER, FBO);

// 颜色纹理附件
glGenTextures(1, &textureColorBuffer);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, ...);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ...);

// 深度+模板RBO
glGenRenderbuffers(1, &RBO);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, ...);
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, ...);
```

#### 6.2 淡入淡出后处理

**Shader实现**：

```glsl
uniform sampler2D scene;
uniform float fadeAlpha;

void main() {
    vec4 color = texture(scene, TexCoords);
    FragColor = vec4(color.rgb, color.a * fadeAlpha);
}
```

**与电影模式联动**：

```cpp
float fadeAlpha = camera.GetCinematicFadeAlpha();
postProcessor->SetFadeAlpha(fadeAlpha);
postProcessor->Render();  // 全屏四边形
```

---

### 7. UI与交互系统

#### 7.1 FreeType文本渲染

```cpp
// 加载字体
FT_Library ft;
FT_Init_FreeType(&ft);
FT_Face face;
FT_New_Face(ft, "assets/fonts/msyh.ttc", 0, &face);
FT_Set_Pixel_Sizes(face, 0, 48);

// 生成字形纹理
for (GLubyte c = 0; c < 128; c++) {
    FT_Load_Char(face, c, FT_LOAD_RENDER);
    GLuint texture;
    glGenTextures(1, &texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 
        face->glyph->bitmap.width, face->glyph->bitmap.rows, ...);
    // 存储字形信息
    Characters.insert(pair<char, Character>(c, character));
}
```

#### 7.2 中文字体支持

- 使用`msyh.ttc`（微软雅黑）或`simsun.ttc`（宋体）
- FreeType加载时指定face index：`FT_New_Face(ft, path, 0, &face)`
- 渲染时逐字符查询字形纹理并拼接

#### 7.3 艺术字动画

```cpp
// F1-F6触发不同效果
if (key == GLFW_KEY_F1) {
    uiManager->ShowArtText("球形烟花", 2.0f, color, AnimType::FadeIn);
}

enum AnimType { FadeIn, Scale, Rainbow, Wave };
void Update(float dt) {
    if (animType == Scale) {
        scale = 1.0f + sin(time * 3.0f) * 0.2f;
    } else if (animType == Rainbow) {
        color = HSVtoRGB(fmod(time, 1.0f), 1.0f, 1.0f);
    }
}
```

---

### 8. 音效系统

#### 8.1 miniaudio引擎

```cpp
ma_engine audioEngine;
ma_engine_init(NULL, &audioEngine);

// 播放音效
void launch(...) {
    int index = rand() % 2;
    string path = "assets/sounds/firework/rise/firework_rise_0" + to_string(index+1) + ".wav";
    ma_engine_play_sound(&audioEngine, path.c_str(), NULL);
}

void createExplosion(...) {
    int index = rand() % 2;
    string path = "assets/sounds/firework/explosion/firework_explosion_0" + to_string(index+1) + ".wav";
    ma_engine_play_sound(&audioEngine, path.c_str(), NULL);
}
```

#### 8.2 音效资源

- 上升音效：`firework_rise_01.wav`, `firework_rise_02.wav`	经过降调减小音量等音频处理
- 爆炸音效：`firework_explosion_01.wav`, `firework_explosion_02.wav`
- 随机选择，避免单调重复

---

### 9. 自动演示系统

#### 9.1 runTest自动发射

**HSV色彩空间生成随机色对**：

```cpp
auto generateRandomColorPair = []() {
    // 主色：HSV全范围随机
    float hue1 = rand();  // 0-1
    float sat1 = rand();
    float val1 = 0.5 + rand()*0.5;  // 0.5-1.0确保不太暗
    vec4 primary = HSVtoRGB(hue1, sat1, val1);
    
    // 辅色：色相偏移-0.2~0.2
    float hue2 = fmod(hue1 + (rand()*0.4-0.2) + 1.0, 1.0);
    float sat2 = clamp(sat1 + rand()*0.3-0.15, 0, 1);
    vec4 secondary = HSVtoRGB(hue2, sat2, val2);
    return {primary, secondary};
};
```

**概率加权选择**：

```cpp
float roll = rand();
if (roll < 0.15) {  // 15%概率
    type = Image;
    skipNextLaunch = true;  // 图片烟花后跳过一帧
} else if (roll < 0.45) {  // 35%概率
    type = Sphere;  // 双色
} else if (roll < 0.80) {  // 35%概率
    type = MultiLayer;  // 三色
} else {  // 15%概率
    type = Heart;  // 单色
}
```

**时间采样**：

```cpp
if (currentTime - lastTestTime < 0.8f) return;  // 每0.8秒发射一次
lastTestTime = currentTime;
```

---

### 10. 资源管理与内存安全

#### 10.1 OpenGL资源清理

```cpp
~PostProcessor() {
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (textureColorBuffer) glDeleteTextures(1, &textureColorBuffer);
    if (RBO) glDeleteRenderbuffers(1, &RBO);
    if (FBO) glDeleteFramebuffers(1, &FBO);
}

// main.cpp中
try {
    if (postProcessor) { delete postProcessor; postProcessor = nullptr; }
    if (skyboxShader) { delete skyboxShader; skyboxShader = nullptr; }
    fireworkSystem.cleanupGL();
    skybox.cleanup();
    ground.cleanup();
    if (uiManager) { uiManager->Cleanup(); delete uiManager; }
} catch (exception& e) {
    cerr << "OpenGL cleanup error: " << e.what() << endl;
}
glfwTerminate();  // 最后销毁上下文
```

#### 10.2 资源加载容错

- **纹理加载失败**：返回默认纹理或跳过，不中断程序
- **模型加载失败**：输出错误日志，使用默认几何体
- **音频初始化失败**：静音运行，不影响视觉效果





## 🐛 五、遇到的问题与解决过程

### 问题1：电影运镜阶段切换的跳变感

**问题描述**：  
初期实现电影模式时，从高空俯瞰切换到环绕飞行时，摄像机位置突然改变，产生明显的"瞬移"效果，破坏了流畅的观影体验。例如在3.5秒时刻，摄像机从（4, 4, 17）突变到（26, 5.5, 22），视角瞬间跳跃。

**根本原因**：  
使用线性插值（`glm::mix`）时，插值因子从0突变到1，导致位置/角度直接跳到目标值，而非平滑过渡。同时，不同阶段的起始位置计算不连贯，前一阶段的结束位置与后一阶段的开始位置存在偏差。

**解决方案**：  

1. **引入smoothstep插值**：

```cpp
float smoothstep(float edge0, float edge1, float x) {
    x = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);  // Hermite插值曲线
}
// 应用于第三阶段
float phase = (t - 8.5f) / 2.5f;
phase = smoothstep(0.0f, 1.0f, phase);
Position = glm::mix(startPos, endPos, phase);
```

2. **添加淡入淡出遮挡**：
   在阶段切换时（3.0-4.0s, 8.0-9.0s），返回alpha值从1.0降至0.0再升至1.0，在后处理shader中应用：

```cpp
float alpha = camera.GetCinematicFadeAlpha();
FragColor = vec4(sceneColor.rgb, alpha);
```

淡出时画面变黑，遮挡位置跳变，淡入时新阶段已就位，用户感知不到瞬移。

3. **确保位置连续性**：
   手动调整各阶段的起始/结束位置，确保前一阶段的`endPos`与后一阶段的`startPos`一致。

**效果**：  
阶段切换变得平滑自然，smoothstep提供了缓入缓出的加速度曲线，淡入淡出进一步掩盖了微小的不连续性，整体观感接近电影级镜头运动。

---

### 问题2：地面雾效与天空盒颜色不匹配

**问题描述**：  
地面雾效使用灰色（0.5, 0.5, 0.5），天空盒亮度为100%，导致地平线处出现明显的色彩断层。远处地面是灰色，但抬头看到的天空却是亮蓝色，缺乏统一的夜景氛围。同时，烟花在远处时被灰色雾效淹没，视觉冲击力下降。

**根本原因**：  

1. 雾效颜色与天空盒底色不协调
2. 天空盒过亮，不符合夜景设定
3. 雾效密度过高，远处物体过快消失

**解决方案**：  

1. **统一颜色基调**：

```glsl
// ground.fs 和 model.fs
vec3 fogColor = vec3(0.02, 0.04, 0.12);  // 深蓝偏黑，匹配夜空
```

2. **降低天空盒亮度**：

```glsl
// skybox.fs
FragColor = texture(skybox, TexCoords) * 0.3;  // 降至30%亮度
```

3. **优化雾效参数**：

```glsl
// 降低密度，延长起始距离
fogDensity = 0.030;  // 原0.05
fogStart = 15.0;     // 原10.0
```

4. **差异化地面与模型雾效**：

```glsl
// 地面雾效更强（更快消失）
float distance = length(FragPos - viewPos);
float fogFactor = exp(-fogDensity * max(0.0, distance - fogStart));

// 模型雾效减半（保持可见）
float modelFogDensity = fogDensity * 0.5;  // model.fs
```

**效果**：  
地平线处颜色平滑过渡，远处地面逐渐融入夜空，烟花在远处仍能保持较高可见度，整体画面协调统一。

---

### 问题3：Assimp加载模型纹理失败

**问题描述**：  
使用Assimp加载书本模型时，模型显示为纯白色或纯黑色，控制台输出"Texture failed to load at path: xxx"。检查发现Assimp解析的纹理路径与实际文件位置不匹配，例如解析出的路径为绝对路径`D:\Models\book.png`，但实际文件在`assets/model/book/textures/book.png`。

**根本原因**：  
不同3D建模软件（Blender/3ds Max/Maya）导出的FBX/OBJ文件，其内嵌的纹理路径格式不一致：

- 有的是绝对路径
- 有的是相对于模型文件的相对路径
- 有的只有裸文件名

Assimp直接使用这些路径加载纹理，但实际文件通常已移动到项目资源目录，导致路径失效。

**解决方案**：  
实现多路径搜索机制：

```cpp
string TextureFromFile(const char* path, const string& directory) {
    string filename = string(path);
    
    // 1. 提取裸文件名（去除路径）
    size_t pos = filename.find_last_of("/\\");
    if (pos != string::npos) {
        filename = filename.substr(pos + 1);
    }
    
    // 2. 按优先级搜索多个路径
    vector<string> searchPaths = {
        directory + "/" + filename,              // 模型目录
        directory + "/textures/" + filename,     // 模型目录/textures
        directory + "/../textures/" + filename,  // 父目录/textures
        filename                                 // 当前目录
    };
    
    for (const auto& testPath : searchPaths) {
        ifstream file(testPath);
        if (file.good()) {
            return testPath;  // 找到第一个存在的文件
        }
    }
    
    cerr << "Texture not found: " << path << endl;
    return "";  // 未找到返回空，后续跳过纹理
}
```

**附加措施**：

- 在模型加载时输出纹理搜索日志，便于调试
- 为模型添加`HasTextures()`方法，shader根据此标志决定是否使用纹理
- 纹理加载失败时不中断程序，使用默认颜色渲染

**效果**：  
成功加载不同来源的模型纹理，兼容性大幅提升，即使纹理路径错误也能通过多路径搜索找到文件。

---

### 问题4：烟花粒子叠加导致颜色过亮

**问题描述**：  
多个烟花同时爆炸时，粒子叠加区域出现过曝效果，颜色变成纯白色，失去了原有的色彩层次。特别是双色二次爆炸时，叠加区域几乎无法辨识烟花类型。

**根本原因**：  
使用加法混合模式（`glBlendFunc(GL_ONE, GL_ONE)`），多个粒子颜色直接相加，超过1.0后被clamp到1.0，导致饱和过曝。

**解决方案**：  

1. **改用标准alpha混合**：

```cpp
glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);  // 标准alpha混合
```

粒子颜色根据alpha值混合，而非直接相加。

2. **降低粒子颜色强度**：

```cpp
// 图片烟花粒子颜色乘以0.2
p.color = pixelColor * 0.2f;

// 普通烟花初始亮度不超过1.0
vec4 color = HSVtoRGB(h, s, v);  // v控制在0.5-1.0
```

3. **优化颜色渐变**：

```cpp
// 延长颜色保持时间，缩短初始爆亮阶段
if (lifeRatio > 0.95f) {  // 只有最初5%时间爆亮
    return initialColor;
} else if (lifeRatio > 0.15f) {  // 中段80%保持原色
    return initialColor;
} else {  // 末段15%快速淡出
    return initialColor * (lifeRatio / 0.15f);
}
```

**效果**：  
粒子叠加时颜色自然混合，保留了色彩层次，画面不再过曝，烟花类型清晰可辨。

---

### 问题5：电影模式与自动测试冲突

**问题描述**：  
同时启动电影模式（按键9）和自动测试（按键0）时，摄像机被自动测试的随机位置覆盖，电影运镜失效。或者电影模式结束后，摄像机位置不恢复，后续操作异常。

**根本原因**：  
两个模式都尝试控制摄像机位置，缺乏优先级管理和互斥逻辑。

**解决方案**：  

1. **添加模式互斥检查**：

```cpp
void ToggleCinematicMode() {
    CinematicMode = !CinematicMode;
    if (CinematicMode) {
        OrbitMode = false;  // 禁用轨道模式
        CinematicTime = 0.0f;
        CinematicInitialPos = Position;  // 保存初始位置
    }
}

void ProcessKeyboard(...) {
    if (CinematicMode || OrbitMode) return;  // 特殊模式下禁止手动控制
}
```

2. **模式结束后恢复状态**：

```cpp
if (t > 11.0f) {
    CinematicMode = false;
    // 可选：恢复到初始位置
    // Position = CinematicInitialPos;
}
```

3. **UI提示当前模式**：

```cpp
if (camera.CinematicMode) {
    uiManager->RenderText("Cinematic Mode Active", ...);
}
```

**效果**：  
模式切换流畅，互不干扰，用户可以清晰知道当前处于哪种模式。

---

### 问题6：图片烟花粒子数过多导致卡顿

**问题描述**：  
加载高分辨率图片（如512x512）时，每个像素都生成一个粒子，导致粒子数超过26万，帧率从60fps骤降到5fps以下，程序几乎卡死。

**根本原因**：  
未设置采样间隔，逐像素生成粒子，粒子数 = 图片分辨率，GPU渲染压力过大。

**解决方案**：  

1. **添加采样间隔**：

```cpp
int sampleRate = 1;  // 每隔1个像素采样
for (int y = 0; y < height; y += sampleRate) {
    for (int x = 0; x < width; x += sampleRate) {
        // 生成粒子
    }
}
```

采样间隔=1时，粒子数为原来的1/1；间隔=3时，粒子数为原来的1/9。

2. **跳过透明像素**：

```cpp
if (pixelColor.a < 0.1f) continue;  // alpha < 0.1视为透明
```

3. **图片缩放适配**：

```cpp
float scale = min(4.0f / width, 4.0f / height);  // 限制在4x4单位内
```

4. **粒子尺寸调整**：

```cpp
p.size = 0.02f;  // 较小的粒子尺寸，避免过度填充
```

**性能对比**：

- 512x512无采样：260K粒子，5fps
- 512x512采样间隔=3：29K粒子，55fps
- 200x200采样间隔=1：40K粒子（跳过透明），60fps（推荐）

**建议**：  
使用200x200以内的PNG图片，采样间隔设为1，跳过透明像素，可获得最佳视觉效果和性能平衡。

---

### 问题7：OpenGL资源清理时程序崩溃

**问题描述**：  
程序退出时，在`glfwTerminate()`后访问OpenGL对象导致访问违例崩溃。或者在清理shader时，由于多次delete同一对象导致double free错误。

**根本原因**：  

1. OpenGL对象必须在上下文销毁前清理
2. 清理顺序错误，先销毁上下文再删除对象
3. 指针未置空，重复delete

**解决方案**：  

1. **严格的清理顺序**：

```cpp
// 1. 先清理OpenGL资源
if (postProcessor) {
    delete postProcessor;
    postProcessor = nullptr;
}
if (skyboxShader) {
    delete skyboxShader;
    skyboxShader = nullptr;
}
fireworkSystem.cleanupGL();
skybox.cleanup();
ground.cleanup();

// 2. 清理UI（包含OpenGL对象）
if (uiManager) {
    uiManager->Cleanup();
    delete uiManager;
    uiManager = nullptr;
}

// 3. 最后销毁GLFW上下文
glfwTerminate();
```

2. **添加异常捕获**：

```cpp
try {
    // 清理代码
} catch (exception& e) {
    cerr << "OpenGL cleanup error: " << e.what() << endl;
}
```

3. **防御性检查**：

```cpp
~PostProcessor() {
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;  // 置零防止重复删除
    }
}
```

4. **标记已清理状态**：

```cpp
bool glInited = false;  // Shader/VAO等对象标志
void cleanupGL() {
    if (!glInited) return;  // 防止重复清理
    // 执行清理
    glInited = false;
}
```

**效果**：  
程序退出时不再崩溃，所有OpenGL资源正确释放，没有内存泄漏。

---

### 问题8：粒子拖尾生命周期过长导致画面模糊

**问题描述**：  
拖尾粒子生命周期设为1.0秒时，屏幕上积累大量拖尾，遮挡了烟花本体，画面变得模糊不清。同时拖尾粒子数量过多（每个主粒子每帧生成一个拖尾），帧率下降。

**根本原因**：  

1. 拖尾寿命过长，消失速度慢
2. 拖尾生成频率过高（每帧生成）
3. 拖尾透明度过高

**解决方案**：  

1. **缩短拖尾寿命**：

```cpp
tail.life = 0.3f;  // 原1.0秒改为0.3秒
tail.maxLife = 0.3f;
```

2. **降低拖尾透明度**：

```cpp
tail.color.a *= 0.5f;  // 半透明
tail.initialColor.a *= 0.5f;
```

3. **拖尾大小衰减**：

```cpp
float t = 1.0f - (tail.life / tail.maxLife);
tail.size = max(0.01f, tail.size * (1.0f - t * 0.05f));
```

4. **条件生成拖尾**：

```cpp
if (!p.isTail && p.velocity.y > 0) {  // 只在上升阶段生成
    createTail(p, prevPos);
}
```

**效果**：  
拖尾清晰可见但不遮挡主体，画面清爽，帧率稳定在60fps。

---

### 总结：核心经验教训

1. **平滑过渡的重要性**：所有涉及连续变化的参数（位置、角度、颜色、透明度）都应使用插值函数（lerp/smoothstep/ease），避免突变。

2. **颜色协调性**：场景中所有元素（天空盒、雾效、光照、粒子）的颜色基调应统一，地平线处尤其需要注意过渡。

3. **路径容错机制**：加载外部资源（纹理、模型、音频）时，应实现多路径搜索和降级方案，提升兼容性。

4. **性能优化意识**：粒子数量、纹理分辨率、绘制调用都会影响性能，应通过采样、剔除、LOD等手段优化。

5. **资源生命周期管理**：OpenGL资源必须在上下文有效期内清理，清理顺序和防御性检查至关重要。

6. **模式互斥与优先级**：多个控制模式（电影/轨道/自由）应互斥，避免冲突，UI应清晰提示当前状态。

7. **调试友好性**：关键操作应输出日志，纹理/模型加载失败应有明确错误信息，便于定位问题。



------

# 🧨 项目构建说明

------

## 🗂 一、项目目录结构

```
Fireworks_OpenGL/
│── Fireworks_OpenGL/
│   │── src/               # 所有 C++ 源文件
│   │── include/           # 所有头文件
│   │── assets/            # 资源
│   │   ├── shaders/
│   │   ├── textures/
│   │   ├── skybox/
│   │   ├── sounds/
│   │   ├── fonts/        # UI用字体文件
│   │   ├── model/         # 3D模型文件
│   │   ├── ground/        # 地面纹理
│   │── external/          # 第三方库（源码形式）
│   │   ├── glfw/
│   │   ├── glad/
│   │   ├── glm/
│── x64/                   # VS 自动生成（已被 gitignore 忽略）
│── Fireworks.sln
│── .gitignore
│── README.md
```

------

## 🛠 二、环境依赖

| 工具                                  | 用途            |
| ------------------------------------- | --------------- |
| **Visual Studio 2022**                | C++17 编译环境  |
| **CMake**                             | 用于编译 GLFW   |
| **vcpkg**                             | 用于安装 Assimp |
| **Win10/Win11 系统自带 OpenGL32.lib** | 无需安装        |

------

## 🧩 三、配置第三方库

### 1. 配置 GLFW

GLFW 以**源码形式**放在：

```
external/glfw/
```

需要手动编译一次。

#### 编译步骤

1. 打开 **CMake GUI**
2. 设置：
   - Source：`external/glfw/glfw-3.4`
   - Build：`external/glfw/glfw-3.4/build`
3. 点击 **Configure**
4. 点击 **Generate**
5. 点击 **Open Project**
6. Visual Studio 中：
   - 选择 **Release | x64**
   - Build → Build Solution（生成 glfw3.lib）

生成位置：

```
external/glfw/glfw-3.4/build/src/Release/glfw3.lib
```

------

### 2. 配置 GLAD（必须）

GLAD 以源码形式放置在：

```
external/glad/include/
external/glad/src/glad.c
```

你不需要编译它，只需要在 VS 中包含 glad.c。

添加方式：

```
右键项目 → Add → Existing Item → external/glad/src/glad.c
```

------

### 3. GLM（自动可用）

下载路径：[Releases · g-truc/glm](https://github.com/g-truc/glm/releases)

GLM 是 header-only，无需编译，直接下载 zip 然后解压到下面这个路径：

```
external/glm/
```

------

### 4. 配置 Assimp——使用vcpkg

#### 🚀 安装步骤

**步骤 1：安装 vcpkg（如果还没有）**

如果你的电脑上还没有 vcpkg，按以下步骤安装：

```powershell
# 1. 克隆 vcpkg 仓库到任意目录（第一行自己选择路径）
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# 2. 运行 bootstrap 脚本
.\bootstrap-vcpkg.bat

# 3. 集成到 Visual Studio（可选但推荐）
.\vcpkg integrate install
```

**步骤 2：安装 Assimp**

```powershell
# 在 vcpkg 目录下运行
.\vcpkg install assimp:x64-windows
```

安装完成后，Assimp 库会被安装到：

```
[vcpkg安装路径]\installed\x64-windows\
├── include\assimp\     # 头文件
├── lib\                # Release 库文件
│   └── assimp-vc143-mt.lib
├── debug\lib\          # Debug 库文件
│   └── assimp-vc143-mtd.lib
└── bin\                # DLL 文件
    └── assimp-vc143-mt.dll
```

**步骤 3：配置 Visual Studio 项目**

打开 Visual Studio，右键项目 → 属性 → 确保配置为 **x64**

**3.1 配置 Include 目录**

在 `C/C++ → General → Additional Include Directories` 中添加：

```
$(ProjectDir)include
$(ProjectDir)external\glfw\glfw-3.4\include
$(ProjectDir)external\glad\include
$(ProjectDir)external\glm
$(VcpkgRoot)installed\x64-windows\include
```

**示例**（假设 vcpkg 在 C:\vcpkg）：

```
C:\vcpkg\installed\x64-windows\include
```

**3.2 配置 Library 目录**

在 `Linker → General → Additional Library Directories` 中添加：

注：下面的所有Debug配置和Release配置的设置只要在左上角切换Release/Debug进行配置即可，如果嫌麻烦可以只配置Release

**Debug 配置**：

```
$(ProjectDir)external\glfw\glfw-3.4\build\src\Release
$(VcpkgRoot)installed\x64-windows\debug\lib
```

**Release 配置**：

```
$(ProjectDir)external\glfw\glfw-3.4\build\src\Release
$(VcpkgRoot)installed\x64-windows\lib
```

**3.3 配置链接库**

在 `Linker → Input → Additional Dependencies` 中添加：

**Debug 配置**：

```
glfw3.lib
opengl32.lib
assimp-vc143-mtd.lib
freetyped.lib
```

**Release 配置**：

```
glfw3.lib
opengl32.lib
assimp-vc143-mt.lib
freetype.lib
```

**步骤 4：复制 DLL 文件**

Assimp 需要 DLL 才能运行，需要将 DLL 复制到可执行文件目录：

在 Visual Studio 中，项目 → 属性 → 生成事件 → 生成后事件 → 命令行，添加：

```cmd
xcopy /y /d "$(VcpkgRoot)\installed\x64-windows\bin\assimp-vc143-mt.dll" "$(OutDir)"
```

（将 `C:\vcpkg` 替换为你的实际 vcpkg 路径）

### 5. 配置 freetype — 使用 vcpkg(已安装)

#### 使用vcpkg安装FreeType

```bash
# 打开PowerShell，进入vcpkg目录
cd C:\vcpkg  # 根据你的实际路径
.\vcpkg install freetype:x64-windows
```

#### 配置Visual Studio项目

在项目属性中添加：

C/C++ → 常规 → 附加包含目录（添加）：

```text
$(VcpkgRoot)\installed\x64-windows\include
```

链接器 → 常规 → 附加库目录（添加）：

Debug配置：

```text
$(VcpkgRoot)\installed\x64-windows\debug\lib
```

Release配置：

```text
$(VcpkgRoot)\installed\x64-windows\lib
```

链接器 → 输入 → 附加依赖项（添加）：

Debug配置：

```text
freetyped.lib
```

Release配置：

```text
freetype.lib
```

------

## ⚙️ 四、配置 Visual Studio（完整版，可用于检查）

右键项目 → 属性 → 配置：Release / Debug、平台：x64

请注意：不要使用绝对路径，这会影响到其他同学，用$()这种宏的方式配置

------

### 📌 1. C/C++ → General → Additional Include Directories

**完整配置**：

```
$(ProjectDir)include
$(ProjectDir)external\glfw\glfw-3.4\include
$(ProjectDir)external\glad\include
$(ProjectDir)external\glm
$(VcpkgRoot)installed\x64-windows\include
```

------

### 📌 2. Linker → General → Additional Library Directories

**Debug 配置**：

```
$(ProjectDir)external\glfw\glfw-3.4\build\src\Release
$(VcpkgRoot)installed\x64-windows\debug\lib
```

**Release 配置**：

```
$(ProjectDir)external\glfw\glfw-3.4\build\src\Release
$(VcpkgRoot)installed\x64-windows\lib
```

------

### 📌 3. Linker → Input → Additional Dependencies

**Debug 配置**：

```
glfw3.lib
opengl32.lib
assimp-vc143-mtd.lib
```

**Release 配置**：

```
glfw3.lib
opengl32.lib
assimp-vc143-mt.lib
```

------

### 📌 4. Build Events → Post-Build Event（推荐）

Command Line：

```cmd
xcopy /y /d "$(VcpkgRoot)\installed\x64-windows\bin\assimp-vc143-mt.dll" "$(OutDir)"
xcopy /y /d "$(VcpkgRoot)\installed\x64-windows\bin\freetype.dll" "$(OutDir)"
```

