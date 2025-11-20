# Fireworks_OpenGL 实现烟花粒子系统
SYSU Computer Graphics Final Project

这个选题主要是关于粒子特效的构建方面，你可以构建一个三维的粒子系统，也可以构建一个二维
的粒子系统（使用的渲染库不限，但不得直接套用游戏引擎库的粒子系统，使用游戏引擎的话，粒子系
统必须是自己构建的）。在本选题中，你将构建一个烟花的粒子系统，烟花的类型由你自己定

参考实现的功能（最好都实现吧）：
- 构建烟花粒子模型：不要求你使用GPU加速，你只需在CPU上实现烟花粒子系统即可
- 添加天空盒：添加一个黑夜的天空盒作为背景
- 添加一个地面：为了使得场景不那么突兀，你只需添加一个平面作为地面
- 实现烟花光照：为了实现烟花爆炸瞬间的光辉璀璨，你需要在每朵烟花的爆炸中心激活一个点光
源，将该点光源的颜色选定为烟花的颜色，爆炸结束后再删除这个点光源。地面需要实现光照（传
统Blinn-Phong光照即可），从而凸显烟火闪烁的效果
- 实现辉光特效：使用高斯模糊之类的后处理技术实现烟花周围的光晕特效，可参考这个
- 添加音效：在烟花爆炸的时候添加一个Boom的音效

参考资料：
- Coding Challenge #27: Fireworks! （需要科学上网）
- Particle System

## 初步想法
做一个多种烟花类型的三维烟花粒子系统，会有多个键位绑定不同烟花类型，后续有时间会加上鼠标点击位置发射烟花功能



## 项目构建



------

### 🗂 项目结构

```
Fireworks_OpenGL/
│── Fireworks_OpenGL/
│   │── src/         （所有 .cpp）
│   │── include/     （所有 .h）
|   │── assets/
|   │   ├── shaders/
|   │   ├── textures/
|   │   ├── skybox/
|   │   ├── sounds/
|   │── external/        （第三方库）
|   │   ├── glfw/
|   │   ├── glad/
|   │   ├── glm/
│── x64/			（不用自己创建）
│── Fireworks.sln
│── .gitignore
│── README.md
```

------

### 📦 安装 GLFW

#### Step 1：下载 GLFW

去官网下载：
 https://www.glfw.org/download.html

下载 **Source package**
 解压到：

```
external/glfw
```

#### Step 2：用 CMake 生成 VS 工程

1. 打开 CMake GUI
2. Source path 设置为 `external/glfw/glfw-3.4`
3. Build path 设置为 `external/glfw/glfw-3.4/build`
4. 点击 **Configure**
5. 点击 **Generate**
6. 打开生成的 `GLFW.sln`
7. 在 VS 中编译为 **Release | x64**，会生成 `glfw3.lib`

输出路径在：

```
external/glfw/glfw-3.4/build/src/Release/glfw3.lib
```

------

### 🧰 安装 GLAD（头文件 + 源文件方式）

#### Step 1：在线生成 GLAD

网站：[https://glad.dav1d.de](https://glad.dav1d.de/)

配置：

- **API：OpenGL 4.1**
- Profile：Core
- Language：C/C++
- 生成（Generate）

点击generate下载 ZIP，解压到：

```
external/glad
```

你会看到：

```
external/glad/include/glad/glad.h
external/glad/src/glad.c
```

------

### 🧩 配置 VS 项目（关键步骤）

右键项目 → 属性 → 设置以下内容：

------

#### ① C/C++ → General → Additional Include Directories

加入：

```
$(ProjectDir)include
$(ProjectDir)external\glfw\glfw-3.4\include
$(ProjectDir)external\glad\include
$(ProjectDir)external\glm
```

------

#### ② Linker → General → Additional Library Directories

加入：

```
$(ProjectDir)external\glfw\build\src\Release
```

------

#### ③ Linker → Input → Additional Dependencies

加入：

```
glfw3.lib
opengl32.lib
```

Windows 上的 OpenGL 驱动库是系统自带的，不需要下载。

------

### ⚙️ 添加 GLAD 源文件

右键 **源文件** → 添加 → 现有项
 添加：

```
external/glad/src/glad.c
```

------

### 🧪 添加主程序 main.cpp（基础框架）

在 `src/main.cpp` 写入如下程序（已测试可运行）：

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // 配置 OpenGL 版本为 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Fireworks Demo", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化 GLAD（必须在 context 后）
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // 背景颜色
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: 在这里渲染地面 / 天空盒 / 粒子系统

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
```

运行后你会看到一个黑色窗口，这就是你们的基础框架。





------

## 🧨 Fireworks OpenGL — 项目构建说明（Clone 后必读）

本项目基于 **OpenGL + GLFW + GLAD + GLM** 实现 3D 烟花粒子特效。
 该说明文档用于指导团队成员在 clone 项目后如何在本地完成构建环境配置。

------

### 🗂 一、项目目录结构

```
Fireworks_OpenGL/
│── Fireworks_OpenGL/
│   │── src/               # 所有 C++ 源文件
│   │── include/           # 所有头文件
│   │── assets/            # 资源（shader、texture、skybox、sound）
│   │   ├── shaders/
│   │   ├── textures/
│   │   ├── skybox/
│   │   ├── sounds/
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

### 🛠 二、环境依赖

| 工具                                  | 用途           |
| ------------------------------------- | -------------- |
| **Visual Studio 2022**                | C++17 编译环境 |
| **CMake**                             | 用于编译 GLFW  |
| **Win10/Win11 系统自带 OpenGL32.lib** | 无需安装       |

------

### 🧩 三、配置第三方库

#### 1. 配置 GLFW（必须）

GLFW 以**源码形式**放在：

```
external/glfw/
```

需要手动编译一次。

#### 编译步骤

1. 打开 **CMake GUI**
2. 设置：
   - Source：`external/glfw/<glfw-version>`
   - Build：`external/glfw/<glfw-version>/build`
3. 点击 **Configure**
4. 点击 **Generate**
5. 点击 **Open Project**
6. Visual Studio 中：
   - 选择 **Release | x64**
   - Build → Build Solution（生成 glfw3.lib）

生成位置：

```
external/glfw/<version>/build/src/Release/glfw3.lib
```

------

#### 2. 配置 GLAD（必须）

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

#### 3. GLM（自动可用）

GLM 是 header-only，无需编译，路径：

```
external/glm/
```

------

### ⚙️ 四、配置 Visual Studio

右键项目 → 属性 → 配置：Release / Debug、平台：x64

------

#### 📌 1. C/C++ → General → Additional Include Directories

添加：

```
$(ProjectDir)include
$(ProjectDir)external\glfw\<version>\include
$(ProjectDir)external\glad\include
$(ProjectDir)external\glm
```

------

#### 📌 2. Linker → General → Additional Library Directories

添加：

```
$(ProjectDir)external\glfw\<version>\build\src\Release
```

------

#### 📌 3. Linker → Input → Additional Dependencies

添加：

```
glfw3.lib
opengl32.lib
```

系统自带 opengl32.lib。

------

### 🧪 五、构建 & 运行

1. Visual Studio 选择：
   - **x64**
   - **Debug** 或 **Release**
2. 点击 Build → Build Solution
3. 运行即可看到窗口 + 你的测试场景（初期是黑色窗口正常）

------

### ✨ 六、常见问题

#### ✔ external/build 目录不提交

已经由 `.gitignore` 自动忽略。

#### ✔ 如果 clone 后无法编译 GLFW

一定是没有运行 CMake
 或 VS 缺少 C++ Desktop Development 组件。

------

### 🎯 七、团队成员如何继续开发

1. clone 本项目
2. 通过 CMake 编译一次 GLFW
3. 设置 VS 项目 include/library 目录
4. 确保 glad.c 被添加
5. 开始编写 src/ 下的模块代码即可

------



## 
