// src/UIManager.cpp
#include "UIManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <glad/glad.h>

// TextElement 方法实现
void TextElement::Render(TextRenderer* textRenderer) {
    if (visible && !text.empty()) {
        textRenderer->RenderTextWithAlpha(text, x, y, scale, color);
    }
}

void TextElement::Update(float deltaTime) {
    // 文本元素通常不需要每帧更新
}

// ButtonElement 方法实现
void ButtonElement::Render(TextRenderer* textRenderer) {
    if (!visible) return;

    // 这里可以绘制按钮背景（需要额外的渲染逻辑）
    // 暂时只绘制文本

    glm::vec4 color = (state == HOVER && active) ? hoverColor : textColor;
    textRenderer->RenderTextWithAlpha(text, x, y, 1.0f, color);
}

void ButtonElement::Update(float deltaTime) {
    // 按钮状态更新
}

// UIManager 方法实现
UIManager::UIManager()
    : textRenderer(nullptr), screenWidth(0), screenHeight(0) {
    currentHint.duration = 0;
    currentHint.timeLeft = 0;
    currentHint.alpha = 0;
}

UIManager::~UIManager() {
    Cleanup();
}

bool UIManager::Initialize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;

    // 初始化文本渲染器
    textRenderer = new TextRenderer(width, height);
    if (!textRenderer) {
        std::cerr << "[UIManager] Failed to create TextRenderer" << std::endl;
        return false;
    }

    // 加载字体
    if (!LoadFonts()) {
        std::cerr << "[UIManager] Font loading failed" << std::endl;
        return false;
    }

    // 创建默认UI元素
    CreateDefaultUI();

    std::cout << "[UIManager] Initialized successfully" << std::endl;
    return true;
}

void UIManager::Cleanup() {
    // 删除所有UI元素
    for (auto& pair : uiElements) {
        delete pair.second;
    }
    uiElements.clear();

    // 删除文本渲染器
    if (textRenderer) {
        delete textRenderer;
        textRenderer = nullptr;
    }
}

bool UIManager::LoadFonts() {
    // 字体文件路径列表
    std::vector<std::string> fontPaths = {
        "assets/fonts/msyh.ttc",
        "assets/fonts/arial.ttf",
        "fonts/msyh.ttc",
        "fonts/arial.ttf"
    };

    for (const auto& path : fontPaths) {
        std::ifstream fontFile(path);
        if (fontFile.good()) {
            fontFile.close();
            if (textRenderer->LoadFont(path, 24)) {
                std::cout << "[UIManager] Loaded font from: " << path << std::endl;
                return true;
            }
        }
    }

    std::cerr << "[UIManager] Could not load any font" << std::endl;
    return false;
}

void UIManager::CreateDefaultUI() {
    // 标题
    TextElement* title = new TextElement();
    title->SetText("烟花系统 Fireworks");
    title->SetPosition(15.0f, 20.0f);
    title->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    title->SetScale(1.0f);
    AddElement("title", title);

    // FPS显示
    TextElement* fps = new TextElement();
    fps->SetText("FPS: 0");
    fps->SetPosition(15.0f, 60.0f);
    fps->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    fps->SetScale(0.8f);
    AddElement("fps", fps);

    // 烟花类型
    TextElement* fireworkType = new TextElement();
    fireworkType->SetText("当前类型: 球形 Sphere");
    fireworkType->SetPosition(15.0f, 90.0f);
    fireworkType->SetColor(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
    fireworkType->SetScale(0.8f);
    AddElement("firework_type", fireworkType);

    // 烟花计数
    TextElement* fireworkCount = new TextElement();
    fireworkCount->SetText("发射次数: 0");
    fireworkCount->SetPosition(15.0f, 120.0f);
    fireworkCount->SetColor(glm::vec4(0.8f, 0.8f, 1.0f, 1.0f));
    fireworkCount->SetScale(0.8f);
    AddElement("firework_count", fireworkCount);

    // 模式显示
    TextElement* mode = new TextElement();
    mode->SetText("模式: 手动 Manual");
    mode->SetPosition(15.0f, 150.0f);
    mode->SetColor(glm::vec4(0.2f, 0.8f, 1.0f, 1.0f));
    mode->SetScale(0.8f);
    AddElement("mode", mode);

    // 鼠标状态
    TextElement* mouseState = new TextElement();
    mouseState->SetText("鼠标: 自由 Free");
    mouseState->SetPosition(15.0f, 180.0f);
    mouseState->SetColor(glm::vec4(1.0f, 1.0f, 0.3f, 1.0f));
    mouseState->SetScale(0.8f);
    AddElement("mouse_state", mouseState);

    // 灯光状态
    TextElement* lightsState = new TextElement();
    lightsState->SetText("场景灯光: 开 ON");
    lightsState->SetPosition(15.0f, 210.0f);
    lightsState->SetColor(glm::vec4(0.3f, 1.0f, 0.3f, 1.0f));
    lightsState->SetScale(0.8f);
    AddElement("lights_state", lightsState);

    // 控制提示标题
    TextElement* controlsTitle = new TextElement();
    controlsTitle->SetText("--- 控制 Controls ---");
    controlsTitle->SetPosition(15.0f, (float)screenHeight - 200.0f);
    controlsTitle->SetColor(glm::vec4(0.9f, 0.9f, 0.9f, 0.8f));
    controlsTitle->SetScale(0.6f);
    AddElement("controls_title", controlsTitle);

    // 控制提示行
    std::vector<std::string> controlHints = {
        "1-6: 选择烟花类型",
        "鼠标点击: 发射烟花",
        "M: 切换鼠标控制",
        "L: 切换场景灯光",
        "R: 环绕模式",
        "F: 聚焦中心",
        "0: 自动测试模式",
        "H: 隐藏/显示提示",
        "ESC: 退出"
    };

    float yPos = screenHeight - 180.0f;
    for (int i = 0; i < (int)controlHints.size(); i++) {
        std::string id = "control_hint_" + std::to_string(i);
        TextElement* hint = new TextElement();
        hint->SetText(controlHints[i]);
        hint->SetPosition(25.0f, yPos);
        hint->SetColor(glm::vec4(0.9f, 0.9f, 0.9f, 0.8f));
        hint->SetScale(0.5f);
        AddElement(id, hint);
        yPos += 18.0f;
    }
}

void UIManager::Render(float deltaTime) {
    if (!textRenderer) return;

    // 更新UI淡入效果
    uiFadeTime += deltaTime;
    float fadeAlpha = std::min(uiFadeTime / 2.0f, 1.0f);

    // 更新提示系统
    if (currentHint.timeLeft > 0) {
        currentHint.timeLeft -= deltaTime;
        currentHint.alpha = std::min(currentHint.timeLeft / 0.5f, 1.0f);
        currentHint.alpha = std::max(currentHint.alpha, 0.0f);
    }

    // 保存OpenGL状态
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);

    // 设置UI渲染状态
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 应用淡入效果到UI元素
    for (auto& pair : uiElements) {
        if (pair.second->GetType() == UIElement::TEXT) {
            TextElement* textElement = static_cast<TextElement*>(pair.second);
            glm::vec4 color = textElement->GetColor();

            // 对主UI面板应用淡入效果
            if (pair.first.find("control_hint_") != std::string::npos ||
                pair.first == "controls_title") {
                // 控制提示根据showControlHints决定
                if (!showControlHints) {
                    textElement->SetVisible(false);
                    continue;
                }
                textElement->SetVisible(true);
            }
            else if (pair.first != "title") {
                // 其他UI元素应用淡入效果
                color.a = fadeAlpha;
                textElement->SetColor(color);
            }
        }

        // 渲染UI元素
        if (pair.second->IsVisible()) {
            pair.second->Update(deltaTime);
            pair.second->Render(textRenderer);
        }
    }

    // 渲染提示
    RenderHint(deltaTime);

    // 恢复OpenGL状态
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    if (!blendEnabled) glDisable(GL_BLEND);
}

void UIManager::RenderHint(float deltaTime) {
    if (currentHint.timeLeft <= 0 || currentHint.alpha <= 0) return;

    glm::vec4 hintColor(1.0f, 0.8f, 0.2f, currentHint.alpha);

    // 粗略估算文本宽度
    float textWidth = currentHint.text.length() * 12.0f;
    float hintX = (screenWidth - textWidth) * 0.5f;

    textRenderer->RenderTextWithAlpha(currentHint.text, hintX, 100.0f, 0.7f, hintColor);
}

void UIManager::UpdateScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;

    if (textRenderer) {
        textRenderer->UpdateProjection(width, height);
    }

    // 更新UI元素位置
    UpdateElementPositions();
}

void UIManager::UpdateElementPositions() {
    // 更新控制提示位置
    UIElement* controlsTitle = GetElement("controls_title");
    if (controlsTitle) {
        controlsTitle->SetPosition(15.0f, (float)screenHeight - 200.0f);
    }

    float yPos = screenHeight - 180.0f;
    for (int i = 0; i < 9; i++) {
        std::string id = "control_hint_" + std::to_string(i);
        UIElement* hint = GetElement(id);
        if (hint) {
            hint->SetPosition(25.0f, yPos);
            yPos += 18.0f;
        }
    }
}

void UIManager::AddElement(const std::string& id, UIElement* element) {
    // 如果已存在同名元素，先删除
    auto it = uiElements.find(id);
    if (it != uiElements.end()) {
        delete it->second;
    }

    uiElements[id] = element;
}

void UIManager::RemoveElement(const std::string& id) {
    auto it = uiElements.find(id);
    if (it != uiElements.end()) {
        delete it->second;
        uiElements.erase(it);
    }
}

UIElement* UIManager::GetElement(const std::string& id) {
    auto it = uiElements.find(id);
    return (it != uiElements.end()) ? it->second : nullptr;
}

TextElement* UIManager::GetTextElement(const std::string& id) {
    UIElement* element = GetElement(id);
    if (element && element->GetType() == UIElement::TEXT) {
        return static_cast<TextElement*>(element);
    }
    return nullptr;
}

void UIManager::ShowHint(const std::string& text, float duration) {
    currentHint.text = text;
    currentHint.duration = duration;
    currentHint.timeLeft = duration;
    currentHint.alpha = 1.0f;

    std::cout << "[UI Hint] " << text << std::endl;
}

void UIManager::IncrementFireworkCount() {
    fireworkCount++;
    SetFireworkCount(fireworkCount);

    std::string hint = "烟花发射! 总数: " + std::to_string(fireworkCount) +
        " / Fireworks: " + std::to_string(fireworkCount);
    ShowHint(hint, 1.5f);

    std::cout << "[UI] Firework count incremented to: " << fireworkCount << std::endl;
}

void UIManager::SetFireworkCount(int count) {
    fireworkCount = count;

    TextElement* element = GetTextElement("firework_count");
    if (element) {
        std::string text = "发射次数: " + std::to_string(count);
        element->SetText(text);
    }
}

void UIManager::SetFireworkType(int type) {
    TextElement* element = GetTextElement("firework_type");
    if (element) {
        std::string chineseNames[] = { "球形", "环形", "心形", "多层", "螺旋", "二次爆炸" };
        std::string englishNames[] = { "Sphere", "Ring", "Heart", "MultiLayer", "Spiral", "Double" };

        if (type >= 1 && type <= 6) {
            std::string text = "当前类型: " + chineseNames[type - 1] + " " + englishNames[type - 1];
            element->SetText(text);

            // 根据类型设置颜色
            glm::vec4 colors[] = {
                glm::vec4(1.0f, 0.5f, 0.5f, 1.0f),  // 红
                glm::vec4(0.5f, 1.0f, 0.5f, 1.0f),  // 绿
                glm::vec4(1.0f, 0.5f, 1.0f, 1.0f),  // 紫
                glm::vec4(0.5f, 0.8f, 1.0f, 1.0f),  // 蓝
                glm::vec4(1.0f, 0.8f, 0.2f, 1.0f),  // 金
                glm::vec4(0.8f, 0.3f, 1.0f, 1.0f)   // 紫
            };
            element->SetColor(colors[type - 1]);

            // 提示信息
            std::string hint = "已选择: " + chineseNames[type - 1] +
                " / Selected: " + englishNames[type - 1];
            ShowHint(hint, 1.5f);

            std::cout << "[UI] Firework type set to: " << type
                << " (" << englishNames[type - 1] << ")" << std::endl;
        }
    }
}

void UIManager::SetFPS(float fps) {
    TextElement* element = GetTextElement("fps");
    if (element) {
        std::string text = "FPS: " + std::to_string((int)fps);
        element->SetText(text);

        // 根据FPS设置颜色
        glm::vec4 color;
        if (fps >= 60.0f) {
            color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);  // 绿色
        }
        else if (fps >= 30.0f) {
            color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);  // 黄色
        }
        else {
            color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);  // 红色
        }
        element->SetColor(color);
    }
}

void UIManager::SetMouseEnabled(bool enabled) {
    TextElement* element = GetTextElement("mouse_state");
    if (element) {
        std::string status = enabled ? "锁定 Locked" : "自由 Free";
        std::string text = "鼠标: " + status;
        element->SetText(text);

        glm::vec4 color = enabled ?
            glm::vec4(0.3f, 1.0f, 0.3f, 1.0f) :  // 绿色
            glm::vec4(1.0f, 1.0f, 0.3f, 1.0f);    // 黄色
        element->SetColor(color);
    }

    std::string status = enabled ? "锁定 / Locked" : "自由 / Free";
    ShowHint("鼠标: " + status + " / Mouse: " + status, 1.5f);
}

void UIManager::SetSceneLightsEnabled(bool enabled) {
    TextElement* element = GetTextElement("lights_state");
    if (element) {
        std::string status = enabled ? "开 ON" : "关 OFF";
        std::string text = "场景灯光: " + status;
        element->SetText(text);

        glm::vec4 color = enabled ?
            glm::vec4(0.3f, 1.0f, 0.3f, 1.0f) :  // 绿色
            glm::vec4(1.0f, 0.3f, 0.3f, 1.0f);    // 红色
        element->SetColor(color);
    }

    std::string status = enabled ? "开 / ON" : "关 / OFF";
    ShowHint("场景灯光: " + status + " / Scene Lights: " + status, 1.5f);
}

void UIManager::SetAutoTestMode(bool enabled) {
    TextElement* element = GetTextElement("mode");
    if (element) {
        std::string mode = enabled ? "自动 Auto" : "手动 Manual";
        std::string text = "模式: " + mode;
        element->SetText(text);

        glm::vec4 color = enabled ?
            glm::vec4(1.0f, 0.5f, 0.2f, 1.0f) :  // 橙色
            glm::vec4(0.2f, 0.8f, 1.0f, 1.0f);    // 青色
        element->SetColor(color);
    }

    if (enabled) {
        ShowHint("自动测试模式开启 / Auto Test Mode ON", 2.0f);
    }
    else {
        ShowHint("自动测试模式关闭 / Auto Test Mode OFF", 2.0f);
    }
}

void UIManager::ToggleControlHints() {
    showControlHints = !showControlHints;

    std::string status = showControlHints ? "显示" : "隐藏";
    std::string hint = "控制提示: " + status + " / Control Hints: " + status;

    ShowHint(hint, 1.0f);

    std::cout << "[UI] Control hints: " << (showControlHints ? "ON" : "OFF") << std::endl;
}

void UIManager::OnMouseMove(float x, float y) {
    // 更新所有按钮的悬停状态
    for (auto& pair : uiElements) {
        if (pair.second->GetType() == UIElement::BUTTON) {
            ButtonElement* button = static_cast<ButtonElement*>(pair.second);
            if (button->Contains(x, y)) {
                button->SetState(ButtonElement::HOVER);
            }
            else {
                button->SetState(ButtonElement::NORMAL);
            }
        }
    }
}

void UIManager::OnMouseClick(float x, float y) {
    // 处理UI元素点击
    for (auto& pair : uiElements) {
        if (pair.second->Contains(x, y) && pair.second->IsActive()) {
            std::cout << "[UI] Clicked on: " << pair.first << std::endl;
            // 这里可以添加点击事件处理
        }
    }
}