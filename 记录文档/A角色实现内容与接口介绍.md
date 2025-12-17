# A角色实现内容与接口介绍

## ?? 角色职责概述

**A角色**负责项目的**基础架构搭建**，包括：
- 摄像机系统
- 天空盒系统
- 地面系统
- 3D模型加载系统
- 点光源管理系统
- 基础渲染框架

---

## ?? 已完成功能列表

### 1. ? 摄像机系统 (`Camera.h`)

#### 功能特性
- **自由移动模式**：支持六自由度移动（前后左右上下）
- **轨道环绕模式**：自动围绕场景中心旋转
- **视角聚焦**：可聚焦到指定点
- **鼠标控制**：支持鼠标视角控制和滚轮缩放

#### 关键接口

```cpp
// 构造函数
Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
       glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
       float yaw = YAW,
       float pitch = PITCH);

// 核心方法
glm::mat4 GetViewMatrix();  // 获取视图矩阵（用于渲染）

// 输入处理
void ProcessKeyboard(Camera_Movement direction, float deltaTime);  // 键盘移动
void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);  // 鼠标视角
void ProcessMouseScroll(float yoffset);  // 滚轮缩放

// 高级功能
void ToggleOrbitMode();  // 切换轨道模式
void UpdateOrbitMode(float deltaTime);  // 更新轨道模式（需在渲染循环中调用）
void FocusOn(glm::vec3 target);  // 聚焦到指定点
void SetOrbitCenter(glm::vec3 center);  // 设置轨道中心

// 公共属性
glm::vec3 Position;  // 当前位置
glm::vec3 Front;     // 前方向
glm::vec3 Up;        // 上方向
float Zoom;          // 视野角度（FOV）
bool OrbitMode;      // 是否处于轨道模式
```

#### 使用示例

```cpp
// 创建摄像机
Camera camera(glm::vec3(0.0f, 4.5f, 10.0f));

// 渲染循环中
void Update(float deltaTime) {
    // 处理输入
    if (键盘W) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (键盘S) camera.ProcessKeyboard(BACKWARD, deltaTime);
    // ... 其他方向
    
    // 更新轨道模式
    camera.UpdateOrbitMode(deltaTime);
    
    // 获取视图矩阵
    glm::mat4 view = camera.GetViewMatrix();
}

// 鼠标回调中
void mouse_callback(double xoffset, double yoffset) {
    camera.ProcessMouseMovement(xoffset, yoffset);
}
```

---

### 2. ? 天空盒系统 (`Skybox.h`)

#### 功能特性
- 支持立方体贴图（Cube Map）
- 支持从6个独立面加载
- 支持等距柱状图加载（简化版）
- 自动深度测试优化

#### 关键接口

```cpp
// 构造函数（自动初始化）
Skybox();

// 加载方法（三选一）
void LoadCubemap(std::vector<std::string> faces);  // 自定义6个面路径
void LoadCubemapSeparateFaces(const std::string& directory);  // 标准命名（px/nx/py/ny/pz/nz.png）
void LoadCubemapFromEquirectangular(const std::string& path);  // 等距柱状图（简化）

// 渲染方法
void Draw();  // 绘制天空盒（需先激活shader并设置uniform）

// OpenGL对象
unsigned int VAO, VBO;
unsigned int cubemapTexture;
```

#### 使用示例

```cpp
// 创建天空盒
Skybox skybox;

// 加载纹理（标准命名方式）
skybox.LoadCubemapSeparateFaces("assets/skybox/NightSkyHDRI007_4K");
// 期望目录下有：px.png, nx.png, py.png, ny.png, pz.png, nz.png

// 渲染循环中（最后渲染）
skyboxShader.use();
glm::mat4 skyboxView = glm::mat4(glm::mat3(view));  // 移除平移分量
skyboxShader.setMat4("view", skyboxView);
skyboxShader.setMat4("projection", projection);
skyboxShader.setInt("skybox", 0);
skybox.Draw();
```

#### 天空盒Shader要求

**顶点着色器 (`skybox.vs`)**:
```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
```

**片段着色器 (`skybox.fs`)**:
```glsl
#version 330 core
out vec4 FragColor;
in vec3 TexCoords;

uniform samplerCube skybox;

void main() {    
    FragColor = texture(skybox, TexCoords);
}
```

---

### 3. ? 地面系统 (`Ground.h`)

#### 功能特性
- 可配置尺寸和位置
- 支持纹理贴图（可选）
- 支持纹理重复控制
- 包含法线信息（用于光照）
- 支持雾效渲染

#### 关键接口

```cpp
// 构造函数
Ground(float groundSize = 50.0f, glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f));

// 纹理加载
void LoadTexture(const std::string& path, float repeat = 20.0f);  // repeat控制纹理重复次数

// 渲染方法
void Draw();  // 绘制地面（需先激活shader并设置uniform）
glm::mat4 GetModelMatrix();  // 获取模型矩阵

// 公共属性
float size;           // 地面尺寸
bool hasTexture;      // 是否已加载纹理
float textureRepeat;  // 纹理重复次数
```

#### 使用示例

```cpp
// 创建地面
Ground ground(100.0f);  // 100x100单位的地面

// 加载纹理（可选）
ground.LoadTexture("assets/ground/sea.png", 20.0f);

// 渲染循环中
groundShader.use();
groundShader.setMat4("projection", projection);
groundShader.setMat4("view", view);
groundShader.setMat4("model", ground.GetModelMatrix());
groundShader.setVec3("viewPos", camera.Position);
groundShader.setVec3("groundColor", glm::vec3(0.2f, 0.25f, 0.3f));  // 默认颜色
groundShader.setBool("useTexture", ground.hasTexture);

// 材质属性
groundShader.setFloat("groundShininess", 32.0f);
groundShader.setFloat("groundSpecularStrength", 0.3f);

// 雾效参数
groundShader.setVec3("fogColor", glm::vec3(0.05f, 0.05f, 0.1f));
groundShader.setFloat("fogDensity", 0.02f);
groundShader.setFloat("fogStart", 30.0f);

// 光源数据（见点光源系统）
// ...

ground.Draw();
```

#### 地面Shader要求

需要实现：
- Blinn-Phong光照模型
- 多点光源支持（最多16个）
- 雾效计算
- 可选纹理采样

详见 `assets/shaders/ground.vs` 和 `ground.fs`

---

### 4. ? 点光源管理系统 (`PointLight.h`)

#### 功能特性
- 支持永久光源（场景照明）
- 支持临时光源（烟花爆炸）
- 临时光源自动衰减和移除
- 最多支持16个光源（与shader一致）
- 生命周期管理

#### 关键接口

```cpp
// 点光源结构体
struct PointLight {
    glm::vec3 position;     // 位置
    glm::vec3 color;        // 颜色
    float intensity;        // 强度
    float lifetime;         // 剩余生命周期（-1表示永久）
    bool isPermanent;       // 是否永久光源
    
    bool Update(float deltaTime);  // 更新（返回false表示死亡）
    bool IsDead() const;           // 检查是否死亡
};

// 点光源管理器
class PointLightManager {
public:
    static const int MAX_LIGHTS = 16;
    
    // 添加光源
    void AddPermanentLight(glm::vec3 position, glm::vec3 color, float intensity);
    void AddTemporaryLight(glm::vec3 position, glm::vec3 color, float intensity, float lifetime);
    
    // 更新与清理
    void Update(float deltaTime);  // 更新所有光源（必须在渲染循环中调用）
    void ClearTemporaryLights();   // 清除所有临时光源
    void ClearAllLights();          // 清除所有光源
    
    // 获取光源数据（用于传递给shader）
    int GetLightCount() const;
    std::vector<glm::vec3> GetPositions() const;
    std::vector<glm::vec3> GetColors() const;
    std::vector<float> GetIntensities() const;
    const std::vector<PointLight>& GetLights() const;
};
```

#### 使用示例

```cpp
// 创建全局光源管理器
PointLightManager lightManager;

// 添加永久场景光源
lightManager.AddPermanentLight(
    glm::vec3(-3.0f, 3.0f, 3.0f),   // 位置
    glm::vec3(1.0f, 0.95f, 0.9f),   // 颜色（暖白）
    3.0f                             // 强度
);

// 烟花爆炸时添加临时光源
void OnFireworkExplosion(glm::vec3 pos, glm::vec3 color) {
    lightManager.AddTemporaryLight(
        pos,      // 爆炸位置
        color,    // 烟花颜色
        15.0f,    // 高强度（产生强烈光照）
        2.0f      // 持续2秒
    );
}

// 渲染循环中
void RenderLoop(float deltaTime) {
    // 1. 更新光源（移除过期光源）
    lightManager.Update(deltaTime);
    
    // 2. 获取光源数据
    int numLights = lightManager.GetLightCount();
    auto lightPositions = lightManager.GetPositions();
    auto lightColors = lightManager.GetColors();
    auto lightIntensities = lightManager.GetIntensities();
    
    // 3. 传递给shader
    shader.use();
    shader.setInt("numLights", numLights);
    for (int i = 0; i < numLights && i < 16; i++) {
        std::string index = std::to_string(i);
        shader.setVec3("lightPositions[" + index + "]", lightPositions[i]);
        shader.setVec3("lightColors[" + index + "]", lightColors[i]);
        shader.setFloat("lightIntensities[" + index + "]", lightIntensities[i]);
    }
}
```

#### Shader要求

**片段着色器中需要定义**：
```glsl
#define MAX_LIGHTS 16
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];
uniform float lightIntensities[MAX_LIGHTS];

// Blinn-Phong光照计算
vec3 CalcPointLight(vec3 lightPos, vec3 lightColor, float intensity, 
                    vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    vec3 lightDir = normalize(lightPos - fragPos);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    
    // 镜面反射（Blinn-Phong）
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    
    // 距离衰减
    float distance = length(lightPos - fragPos);
    float attenuation = intensity / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    
    // 组合
    vec3 ambient = 0.02 * lightColor;
    vec3 diffuse = diff * lightColor * 0.8;
    vec3 specular = spec * lightColor * specularStrength;
    
    return (ambient + diffuse + specular) * attenuation * baseColor;
}

// 在main()中遍历所有光源
for(int i = 0; i < numLights && i < MAX_LIGHTS; i++) {
    result += CalcPointLight(lightPositions[i], lightColors[i], 
                            lightIntensities[i], norm, FragPos, viewDir, color);
}
```

---

### 5. ? 3D模型加载系统 (`Model.h`, `Mesh.h`)

#### 功能特性
- 使用Assimp加载多种格式（FBX/GLTF/OBJ等）
- 自动处理网格、材质、纹理
- 智能纹理路径查找
- 支持多网格模型
- 法线自动生成

#### 关键接口

```cpp
// Model类（在Model.h中）
class Model {
public:
    // 构造函数（自动加载模型）
    Model(std::string const& path, bool gamma = false);
    
    // 渲染方法
    void Draw(Shader& shader);  // 绘制整个模型（所有网格）
    
    // 工具方法
    bool HasTextures() const;  // 检查是否加载了纹理
    
    // 数据
    std::vector<Mesh> meshes;           // 所有网格
    std::vector<Texture> textures_loaded;  // 已加载的纹理
    std::string directory;              // 模型目录
};

// Mesh类（在Mesh.h中）
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture {
    unsigned int id;
    std::string type;  // "texture_diffuse", "texture_specular", etc.
    std::string path;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, 
         std::vector<Texture> textures);
    void Draw(Shader& shader);
};
```

#### 使用示例

```cpp
// 加载模型
Model bookModel("assets/model/book/source/TEST2.fbx");

// 渲染循环中
modelShader.use();
modelShader.setMat4("projection", projection);
modelShader.setMat4("view", view);

// 设置模型变换
glm::mat4 modelMatrix = glm::mat4(1.0f);
modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 3.0f, 0.0f));  // 位置
modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));  // 旋转
modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 0.25f));  // 缩放
modelShader.setMat4("model", modelMatrix);

// 设置光照和材质
modelShader.setVec3("viewPos", camera.Position);
modelShader.setBool("hasTexture", bookModel.HasTextures());
modelShader.setFloat("materialShininess", 64.0f);
modelShader.setFloat("specularStrength", 0.5f);

// 设置光源数据
// ...

// 绘制
bookModel.Draw(modelShader);
```

#### 模型Shader要求

**顶点着色器**：
```glsl
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

**片段着色器**：
- 支持 `texture_diffuse1` 纹理采样
- 实现 Blinn-Phong 光照
- 支持多点光源
- 材质属性：`materialShininess`, `specularStrength`

---

### 6. ? 着色器工具类 (`Shader.h`)

#### 功能特性
- 自动加载和编译着色器
- 错误检查和报告
- 便捷的uniform设置方法
- 支持所有常用uniform类型

#### 关键接口

```cpp
class Shader {
public:
    unsigned int ID;  // 着色器程序ID
    
    // 构造函数（自动加载和编译）
    Shader(const char* vertexPath, const char* fragmentPath);
    
    // 激活着色器
    void use();
    
    // Uniform设置方法
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
};
```

#### 使用示例

```cpp
// 加载着色器
Shader myShader("assets/shaders/vertex.vs", "assets/shaders/fragment.fs");

// 渲染时使用
myShader.use();

// 设置uniform
myShader.setMat4("projection", projection);
myShader.setMat4("view", view);
myShader.setMat4("model", modelMatrix);
myShader.setVec3("viewPos", camera.Position);
myShader.setFloat("time", glfwGetTime());
myShader.setBool("useTexture", true);
myShader.setInt("texture1", 0);
```

---

## ?? 系统集成示例

### 完整渲染循环

```cpp
int main() {
    // 1. 初始化GLFW和OpenGL
    glfwInit();
    // ... 创建窗口、加载GLAD等
    
    // 2. 创建所有对象
    Camera camera(glm::vec3(0.0f, 4.5f, 10.0f));
    Skybox skybox;
    Ground ground(100.0f);
    Model model("assets/model/book/source/TEST2.fbx");
    PointLightManager lightManager;
    
    // 3. 加载资源
    skybox.LoadCubemapSeparateFaces("assets/skybox/NightSkyHDRI007_4K");
    ground.LoadTexture("assets/ground/sea.png");
    
    // 4. 添加场景光源
    lightManager.AddPermanentLight(glm::vec3(-3.0f, 3.0f, 3.0f), 
                                   glm::vec3(1.0f, 0.95f, 0.9f), 3.0f);
    // ... 更多光源
    
    // 5. 加载着色器
    Shader skyboxShader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    Shader groundShader("assets/shaders/ground.vs", "assets/shaders/ground.fs");
    Shader modelShader("assets/shaders/model.vs", "assets/shaders/model.fs");
    
    // 6. 渲染循环
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // 处理输入
        processInput(window);
        
        // 更新
        lightManager.Update(deltaTime);
        camera.UpdateOrbitMode(deltaTime);
        
        // 清屏
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 计算矩阵
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                                (float)SCR_WIDTH / SCR_HEIGHT, 
                                                0.1f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
        // 获取光源数据
        int numLights = lightManager.GetLightCount();
        auto lightPositions = lightManager.GetPositions();
        auto lightColors = lightManager.GetColors();
        auto lightIntensities = lightManager.GetIntensities();
        
        // 绘制地面
        groundShader.use();
        groundShader.setMat4("projection", projection);
        groundShader.setMat4("view", view);
        groundShader.setMat4("model", ground.GetModelMatrix());
        // ... 设置其他uniform
        ground.Draw();
        
        // 绘制模型
        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        // ... 设置模型变换和光照
        model.Draw(modelShader);
        
        // 绘制天空盒（最后）
        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        skybox.Draw();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
```

---

## ?? 文件结构

```
Fireworks_OpenGL/
├── include/
│   ├── Camera.h          ? 摄像机系统
│   ├── Skybox.h          ? 天空盒系统
│   ├── Ground.h          ? 地面系统
│   ├── Model.h           ? 模型加载系统
│   ├── Mesh.h            ? 网格类
│   ├── PointLight.h      ? 点光源管理
│   ├── Shader.h          ? 着色器工具
│   └── stb_image.h       ? 图像加载库
├── assets/
│   ├── shaders/
│   │   ├── skybox.vs/fs      ? 天空盒着色器
│   │   ├── ground.vs/fs      ? 地面着色器
│   │   └── model.vs/fs       ? 模型着色器
│   ├── skybox/               ? 天空盒纹理
│   ├── ground/               ? 地面纹理
│   └── model/                ? 3D模型文件
├── main.cpp              ? 主程序入口
└── stb_image_impl.cpp    ? stb_image实现文件
```

---

## ?? 控制说明

| 按键 | 功能 |
|------|------|
| **WASD** | 移动摄像机（前后左右）|
| **Space** | 摄像机上升 |
| **Shift** | 摄像机下降 |
| **鼠标移动** | 旋转视角（需启用）|
| **鼠标滚轮** | 缩放（调整FOV）|
| **M** | 切换鼠标控制（锁定/自由）|
| **R** | 切换轨道环绕模式 |
| **F** | 聚焦到场景中心 |
| **T** | 测试：添加临时光源 |
| **ESC** | 退出程序 |

---

## ?? 给其他组员的接口

### 给B角色（粒子逻辑）的接口

```cpp
// 1. 摄像机位置和方向
glm::vec3 cameraPos = camera.Position;
glm::vec3 cameraFront = camera.Front;

// 2. 添加烟花爆炸光源
void OnFireworkExplosion(glm::vec3 position, glm::vec3 color, float intensity, float duration) {
    lightManager.AddTemporaryLight(position, color, intensity, duration);
}

// 3. 世界坐标转换（投影矩阵和视图矩阵）
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                        (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 200.0f);
glm::mat4 view = camera.GetViewMatrix();
```

### 给C角色（渲染/光照）的接口

```cpp
// 1. 光源数据获取
int numLights = lightManager.GetLightCount();
std::vector<glm::vec3> positions = lightManager.GetPositions();
std::vector<glm::vec3> colors = lightManager.GetColors();
std::vector<float> intensities = lightManager.GetIntensities();

// 2. Shader工具
Shader myShader("path/to/vertex.vs", "path/to/fragment.fs");
myShader.use();
myShader.setVec3("uniformName", value);
// ... 其他uniform设置

// 3. 摄像机位置（用于光照计算）
glm::vec3 viewPos = camera.Position;
```

### 给D角色（交互/UI）的接口

```cpp
// 1. 摄像机控制
camera.ProcessKeyboard(FORWARD, deltaTime);
camera.ProcessMouseMovement(xoffset, yoffset);
camera.ProcessMouseScroll(yoffset);

// 2. 摄像机状态查询
bool isOrbitMode = camera.OrbitMode;
glm::vec3 currentPos = camera.Position;
float currentZoom = camera.Zoom;

// 3. 光源管理
int activeLightCount = lightManager.GetLightCount();
// 可用于UI显示当前光源数量
```

### 给E角色（音效/资源）的接口

```cpp
// 1. 光源事件监听
// 当添加临时光源时，播放爆炸音效
void AddTemporaryLightWithSound(glm::vec3 pos, glm::vec3 color, float intensity, float duration) {
    lightManager.AddTemporaryLight(pos, color, intensity, duration);
    // E角色在这里添加音效播放
    PlayExplosionSound();
}

// 2. 资源路径约定
// 天空盒：assets/skybox/
// 地面纹理：assets/ground/
// 模型：assets/model/
// 音效：assets/sounds/（由E角色创建）
```

---

## ?? 已知问题和限制

1. **性能限制**：最多支持16个点光源（受shader uniform数组大小限制）
2. **模型加载**：仅测试了FBX格式，其他格式可能需要调整纹理路径查找逻辑
3. **天空盒**：等距柱状图加载为简化实现，建议使用6个独立面
4. **雾效**：参数需要根据场景调整，当前参数适合中等大小场景

---

## ?? 参考资料

- **OpenGL教程**: [LearnOpenGL CN](https://learnopengl-cn.github.io/)
- **Assimp文档**: [Assimp Documentation](https://assimp.org/lib_html/index.html)
- **GLM文档**: [GLM Manual](https://glm.g-truc.net/0.9.9/index.html)

---

## ? 完成状态

| 功能模块 | 状态 | 完成度 |
|---------|------|--------|
| 摄像机系统 | ? | 100% |
| 天空盒系统 | ? | 100% |
| 地面系统 | ? | 100% |
| 模型加载 | ? | 100% |
| 点光源管理 | ? | 100% |
| 着色器工具 | ? | 100% |
| 基础框架 | ? | 100% |

---

## ?? 联系方式

如有任何问题或需要进一步说明，请联系A角色负责人。

---

**最后更新**: 2024年
**版本**: 1.0
**作者**: A角色（主架构）
