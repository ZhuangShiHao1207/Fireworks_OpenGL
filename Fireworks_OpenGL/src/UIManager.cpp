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

    // 初始化艺术字
    InitializeArtTexts();

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
        "assets/fonts/simhei.ttf",
        "assets/fonts/simsun.ttc",
        "assets/fonts/msyh.ttc",
        "assets/fonts/arial.ttf",
    };

    for (const auto& path : fontPaths) {
        std::ifstream fontFile(path);
        if (fontFile.good()) {
            fontFile.close();
            // 使用较大的字体大小（48）来确保中文清晰
            if (textRenderer->LoadFont(path, 48)) {
                std::cout << "[UIManager] Loaded Chinese font from: " << path << std::endl;
                return true;
            }
        }
    }

    std::cerr << "[UIManager] Could not load any font" << std::endl;
    return false;
}

void UIManager::CreateDefaultUI() {
    // 标题 - 顶部居中
    TextElement* title = new TextElement();
    title->SetText("Fireworks System");
    title->SetPosition(screenWidth / 2.0f - 180.0f, 40.0f); // 粗略居中，后面会精确计算
    title->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    title->SetScale(1.0f);
    AddElement("title", title);

    // ============================================
    // 左侧状态栏 - 左上角（原来标题的位置）
    // ============================================

    // FPS显示
    TextElement* fps = new TextElement();
    fps->SetText("FPS: 0");
    fps->SetPosition(20.0f, 40.0f); // 从标题原来的位置开始
    fps->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    fps->SetScale(0.8f);
    AddElement("fps", fps);

    // 烟花类型
    TextElement* fireworkType = new TextElement();
    fireworkType->SetText("Type: Sphere");
    fireworkType->SetPosition(20.0f, 80.0f);
    fireworkType->SetColor(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
    fireworkType->SetScale(0.8f);
    AddElement("firework_type", fireworkType);

    // 烟花计数
    TextElement* fireworkCount = new TextElement();
    fireworkCount->SetText("Count: 0");
    fireworkCount->SetPosition(20.0f, 120.0f);
    fireworkCount->SetColor(glm::vec4(0.8f, 0.8f, 1.0f, 1.0f));
    fireworkCount->SetScale(0.8f);
    AddElement("firework_count", fireworkCount);

    // 模式显示
    TextElement* mode = new TextElement();
    mode->SetText("Mode: Manual");
    mode->SetPosition(20.0f, 160.0f);
    mode->SetColor(glm::vec4(0.2f, 0.8f, 1.0f, 1.0f));
    mode->SetScale(0.8f);
    AddElement("mode", mode);

    // 鼠标状态
    TextElement* mouseState = new TextElement();
    mouseState->SetText("Mouse: Free");
    mouseState->SetPosition(20.0f, 200.0f);
    mouseState->SetColor(glm::vec4(1.0f, 1.0f, 0.3f, 1.0f));
    mouseState->SetScale(0.8f);
    AddElement("mouse_state", mouseState);

    // 灯光状态
    TextElement* lightsState = new TextElement();
    lightsState->SetText("Lights: ON");
    lightsState->SetPosition(20.0f, 240.0f);
    lightsState->SetColor(glm::vec4(0.3f, 1.0f, 0.3f, 1.0f));
    lightsState->SetScale(0.8f);
    AddElement("lights_state", lightsState);

    // ============================================
    // 烟花类型说明 - 放在状态栏下方
    // ============================================

    // 烟花类型说明标题
    TextElement* fireworkTypesTitle = new TextElement();
    fireworkTypesTitle->SetText("--- Firework Types ---");
    fireworkTypesTitle->SetPosition(20.0f, 300.0f);
    fireworkTypesTitle->SetColor(glm::vec4(1.0f, 1.0f, 0.5f, 1.0f));
    fireworkTypesTitle->SetScale(0.7f);
    AddElement("firework_types_title", fireworkTypesTitle);

    // 烟花类型说明
    std::vector<std::string> fireworkTypeHints = {
        "1: Sphere (Red/Yellow)",
        "2: Ring (Cyan)",
        "3: Heart (Pink)",
        "4: MultiLayer (Blue)",
        "5: Spiral (Gold)",
        "6: Double (Purple)"
    };

    float fireworkHintY = 330.0f;
    float fireworkHintLineHeight = 25.0f;

    for (int i = 0; i < (int)fireworkTypeHints.size(); i++) {
        std::string id = "firework_hint_" + std::to_string(i);
        TextElement* hint = new TextElement();
        hint->SetText(fireworkTypeHints[i]);
        hint->SetPosition(30.0f, fireworkHintY + i * fireworkHintLineHeight);

        // 根据烟花类型设置颜色
        glm::vec4 color;
        switch (i) {
        case 0: color = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f); break;    // 红/黄
        case 1: color = glm::vec4(0.5f, 1.0f, 1.0f, 1.0f); break;    // 青
        case 2: color = glm::vec4(1.0f, 0.5f, 1.0f, 1.0f); break;    // 粉
        case 3: color = glm::vec4(0.3f, 0.8f, 1.0f, 1.0f); break;    // 蓝
        case 4: color = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f); break;    // 金
        case 5: color = glm::vec4(0.8f, 0.3f, 1.0f, 1.0f); break;    // 紫
        default: color = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
        }

        hint->SetColor(color);
        hint->SetScale(0.5f);
        AddElement(id, hint);
    }

    // ============================================
    // 控制说明 - 放在右上角
    // ============================================

    // 控制说明标题 - 右上角
    TextElement* controlsTitle = new TextElement();
    controlsTitle->SetText("--- Controls ---");
    controlsTitle->SetPosition(screenWidth - 325.0f, 60.0f); // 从右上角往下一点开始
    controlsTitle->SetColor(glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));
    controlsTitle->SetScale(0.7f);
    AddElement("controls_title", controlsTitle);

    // 控制说明
    std::vector<std::string> controlHints = {
        "Mouse click: Launch firework",
        "Mouse (locked): ",
        "Left click = Launch",
        "Mouse (locked): ",
        "Right click = Change type",
        "M: Toggle mouse control",
        "L: Toggle scene lights",
        "R: Orbit mode",
        "F: Focus center",
        "0: Auto test mode",
        "H: Hide/Show hints",
        "ESC: Exit"
    };

    float controlHintY = 85.0f;
    float controlHintLineHeight = 25.0f;

    for (int i = 0; i < (int)controlHints.size(); i++) {
        std::string id = "control_hint_" + std::to_string(i);
        TextElement* hint = new TextElement();
        hint->SetText(controlHints[i]);
        hint->SetPosition(screenWidth - 350.0f, controlHintY + i * controlHintLineHeight);
        hint->SetColor(glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));
        hint->SetScale(0.5f);
        AddElement(id, hint);
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

    // 更新艺术字动画
    for (auto& artText : artTexts) {
        if (artText.active) {
            UpdateArtText(artText, deltaTime);
        }
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

            // 对状态面板应用淡入效果
            if (pair.first == "fps" || pair.first == "firework_type" ||
                pair.first == "firework_count" || pair.first == "mode" ||
                pair.first == "mouse_state" || pair.first == "lights_state") {
                color.a = fadeAlpha;
                textElement->SetColor(color);
            }

            // 对烟花类型说明应用可见性控制
            else if (pair.first.find("firework_hint_") != std::string::npos ||
                pair.first == "firework_types_title") {
                if (!showControlHints) {
                    textElement->SetVisible(false);
                    continue;
                }
                textElement->SetVisible(true);
                color.a = fadeAlpha;
                textElement->SetColor(color);
            }

            // 对控制说明应用可见性控制
            else if (pair.first.find("control_hint_") != std::string::npos ||
                pair.first == "controls_title") {
                if (!showControlHints) {
                    textElement->SetVisible(false);
                    continue;
                }
                textElement->SetVisible(true);
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

    // 渲染艺术字（在普通UI之上）
    for (const auto& artText : artTexts) {
        RenderArtText(artText);
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

//计算缩放
float UIManager::CalculateUIScale() const {
    // 基于高度计算缩放
    float baseHeight = 720.0f; // 基准高度
    return std::min(1.0f, static_cast<float>(screenHeight) / baseHeight);
}

void UIManager::UpdateSimpleUIPositions() {
    float scale = CalculateUIScale();

    // 更新UI元素位置和大小
    auto updateElement = [&](const std::string& id, float x, float y, float baseScale) {
        UIElement* element = GetElement(id);
        if (element) {
            element->SetPosition(x * scale, y * scale);
            if (element->GetType() == UIElement::TEXT) {
                TextElement* text = static_cast<TextElement*>(element);
                text->SetScale(baseScale * scale);
            }
        }
        };

    // 标题
    updateElement("title", screenWidth / 2.0f - 180.0f * scale, 40.0f * scale, 1.0f);

    // 左侧状态栏
    float leftX = 20.0f * scale;
    float stateY = 40.0f * scale;
    float stateSpacing = 40.0f * scale;
    updateElement("fps", leftX, stateY, 0.8f);
    updateElement("firework_type", leftX, stateY + stateSpacing, 0.8f);
    updateElement("firework_count", leftX, stateY + stateSpacing * 2, 0.8f);
    updateElement("mode", leftX, stateY + stateSpacing * 3, 0.8f);
    updateElement("mouse_state", leftX, stateY + stateSpacing * 4, 0.8f);
    updateElement("lights_state", leftX, stateY + stateSpacing * 5, 0.8f);

    // 烟花类型说明
    float typesY = stateY + stateSpacing * 7;
    updateElement("firework_types_title", leftX, typesY, 0.7f);
    for (int i = 0; i < 6; i++) {
        updateElement("firework_hint_" + std::to_string(i),
            leftX + 10.0f * scale,
            typesY + 30.0f * scale + i * 25.0f * scale,
            0.5f);
    }

    // 控制说明（右上角）
    float rightX = screenWidth - 350.0f * scale;
    float controlsY = 60.0f * scale;
    updateElement("controls_title", rightX, controlsY, 0.7f);
    for (int i = 0; i < 12; i++) {
        updateElement("control_hint_" + std::to_string(i),
            rightX,
            controlsY + 25.0f * scale + i * 25.0f * scale,
            0.5f);
    }
}


void UIManager::UpdateScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;

    if (textRenderer) {
        textRenderer->UpdateProjection(width, height);
    }

    // 更新UI位置
    UpdateSimpleUIPositions();

    // 更新艺术字位置和大小
    float scale = CalculateUIScale();
    for (auto& artText : artTexts) {
        artText.x = screenWidth / 2.0f;
        artText.y = screenHeight / 2.0f;
        artText.scale = 2.0f * scale;
    }
}

void UIManager::UpdateElementPositions() {
    // 更新标题位置（顶部居中）
    UIElement* title = GetElement("title");
    if (title) {
        // 粗略估算标题宽度："Fireworks System" 大约 17个字符 * 12像素 = 204像素
        float titleWidth = 17.0f * 12.0f;
        float titleX = (screenWidth - titleWidth) / 2.0f;
        title->SetPosition(titleX, 20.0f);
    }
    
    // 更新烟花类型说明位置
    UIElement* fireworkTypesTitle = GetElement("firework_types_title");
    if (fireworkTypesTitle) {
        fireworkTypesTitle->SetPosition(20.0f, 250.0f);
    }
    
    float fireworkHintY = 275.0f;
    float fireworkHintLineHeight = 18.0f;
    
    for (int i = 0; i < 6; i++) {
        std::string id = "firework_hint_" + std::to_string(i);
        UIElement* hint = GetElement(id);
        if (hint) {
            hint->SetPosition(30.0f, fireworkHintY + i * fireworkHintLineHeight);
        }
    }
    
    // 更新控制说明位置（右上角）
    UIElement* controlsTitle = GetElement("controls_title");
    if (controlsTitle) {
        controlsTitle->SetPosition(screenWidth - 200.0f, 60.0f);
    }
    
    float controlHintY = 85.0f;
    float controlHintLineHeight = 18.0f;
    
    for (int i = 0; i < 8; i++) {
        std::string id = "control_hint_" + std::to_string(i);
        UIElement* hint = GetElement(id);
        if (hint) {
            hint->SetPosition(screenWidth - 200.0f, controlHintY + i * controlHintLineHeight);
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

void UIManager::ShowHint(const std::string& text, float duration) {//暂时屏蔽HINT消息
    //currentHint.text = text;
    //currentHint.duration = duration;
    //currentHint.timeLeft = duration;
    //currentHint.alpha = 1.0f;

    //std::cout << "[UI Hint] " << text << std::endl;
}

void UIManager::IncrementFireworkCount() {
    fireworkCount++;
    SetFireworkCount(fireworkCount);

    std::string hint = "Firework launched! Count: " + std::to_string(fireworkCount) +
        " / Fireworks: " + std::to_string(fireworkCount);
    ShowHint(hint, 1.5f);

    //std::cout << "[UI] Firework count incremented to: " << fireworkCount << std::endl;
}

void UIManager::SetFireworkCount(int count) {
    fireworkCount = count;

    TextElement* element = GetTextElement("firework_count");
    if (element) {
        std::string text = "Launch Count: " + std::to_string(count);
        element->SetText(text);
    }
}

void UIManager::SetFireworkType(int type) {
    TextElement* element = GetTextElement("firework_type");
    if (element) {
        std::string englishNames[] = { "Sphere", "Ring", "Heart", "MultiLayer", "Spiral", "Double" };

        if (type >= 1 && type <= 6) {
            std::string text = "Type: " + englishNames[type - 1];
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
            std::string hint = "Selected: " + englishNames[type - 1];
            ShowHint(hint, 1.5f);

            //std::cout << "[UI] Firework type set to: " << type
            //    << " (" << englishNames[type - 1] << ")" << std::endl;
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
        std::string status = enabled ? "Locked" : "Free";
        std::string text = "Mouse: " + status;
        element->SetText(text);

        glm::vec4 color = enabled ?
            glm::vec4(0.3f, 1.0f, 0.3f, 1.0f) :  // 绿色
            glm::vec4(1.0f, 1.0f, 0.3f, 1.0f);    // 黄色
        element->SetColor(color);
    }

    std::string status = enabled ? "Locked" : "Free";
    ShowHint("Mouse: " + status, 1.5f);
}

void UIManager::SetSceneLightsEnabled(bool enabled) {
    TextElement* element = GetTextElement("lights_state");
    if (element) {
        std::string status = enabled ? "ON" : "OFF";
        std::string text = "lights: " + status;
        element->SetText(text);

        glm::vec4 color = enabled ?
            glm::vec4(0.3f, 1.0f, 0.3f, 1.0f) :  // 绿色
            glm::vec4(1.0f, 0.3f, 0.3f, 1.0f);    // 红色
        element->SetColor(color);
    }

    std::string status = enabled ? "ON" : "OFF";
    ShowHint("Scene Lights: " + status, 1.5f);
}

void UIManager::SetAutoTestMode(bool enabled) {
    TextElement* element = GetTextElement("mode");
    if (element) {
        std::string mode = enabled ? "Auto" : "Manual";
        std::string text = "Mode: " + mode;
        element->SetText(text);

        glm::vec4 color = enabled ?
            glm::vec4(1.0f, 0.5f, 0.2f, 1.0f) :  // 橙色
            glm::vec4(0.2f, 0.8f, 1.0f, 1.0f);    // 青色
        element->SetColor(color);
    }

    if (enabled) {
        ShowHint("Auto Test Mode ON", 2.0f);
    }
    else {
        ShowHint("Auto Test Mode OFF", 2.0f);
    }
}

void UIManager::ToggleAllText() {
    showControlHints = !showControlHints;

    // 隐藏/显示所有文本元素（不仅仅是控制提示）
    for (auto& pair : uiElements) {
        // 所有文本元素都受控制
        if (pair.second->GetType() == UIElement::TEXT) {
            pair.second->SetVisible(showControlHints);
        }
    }

    std::string status = showControlHints ? "Shown" : "Hidden";
    std::string hint = "All UI text " + status;

    ShowHint(hint, 1.0f);

    //std::cout << "[UI] All UI text: " << (showControlHints ? "ON" : "OFF") << std::endl;
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
            //std::cout << "[UI] Clicked on: " << pair.first << std::endl;
            // 这里可以添加点击事件处理
        }
    }
}

void UIManager::InitializeArtTexts() {
    // 清空现有艺术字
    artTexts.clear();

    // 6句中文艺术字，对应F1-F6
    // 注意：这里使用UTF-8编码的中文字符串
    std::vector<std::string> messages = {
        u8"烟花粒子系统展示",
        u8"1、展示天空盒、地面与模型",
        u8"2、单独展示部分烟花",
        u8"3、随机发射烟花",
        u8"4、点击地面发射烟花",
        u8"感谢观看",
    };

    // 对应的颜色
    std::vector<glm::vec4> colors = {
        glm::vec4(1.0f, 0.5f, 0.2f, 1.0f),   // 橙色
        glm::vec4(0.2f, 0.8f, 1.0f, 1.0f),   // 蓝色
        glm::vec4(1.0f, 0.8f, 0.2f, 1.0f),   // 金色
        glm::vec4(0.8f, 0.2f, 1.0f, 1.0f),   // 紫色
        glm::vec4(0.2f, 1.0f, 0.5f, 1.0f),   // 绿色
        glm::vec4(1.0f, 0.8f, 0.2f, 1.0f)   // 金色
    };

    // 初始化艺术字对象
    for (int i = 0; i < 6; i++) {
        ArtTextInfo artText;
        artText.text = messages[i];
        artText.color = colors[i];
        artText.x = screenWidth / 2.0f;
        artText.y = screenHeight / 2.0f - 100.0f; // 屏幕中央偏上
        artText.scale = 2.0f;  // 大号字体
        artText.alpha = 0.0f;
        artText.time = 0.0f;
        artText.state = 0;  // 隐藏
        artText.active = false;
        artTexts.push_back(artText);
    }

    std::cout << "[UIManager] Art texts initialized" << std::endl;
}

void UIManager::StartArtTextAnimation(int index) {
    if (index < 0 || index >= artTexts.size()) {
        std::cerr << "[UIManager] Invalid art text index: " << index << std::endl;
        return;
    }

    // 隐藏当前显示的艺术字（如果有）
    for (auto& artText : artTexts) {
        if (artText.active && artText.state != 0) {
            artText.state = 3; // 开始退出动画
            artText.time = 0.0f;
        }
    }

    // 启动选中的艺术字
    ArtTextInfo& artText = artTexts[index];
    artText.state = 1; // 进入动画
    artText.active = true;
    artText.time = 0.0f;
    artText.alpha = 0.0f;
    artText.scale = 2.5f; // 初始放大效果

    // 重新计算居中位置（考虑窗口大小可能已改变）
    artText.x = screenWidth / 2.0f;
    artText.y = screenHeight / 2.0f - 100.0f;

    std::cout << "[UIManager] Art text animation started: " << index + 1 << std::endl;
}

void UIManager::UpdateArtText(ArtTextInfo& artText, float deltaTime) {
    if (!artText.active) return;

    artText.time += deltaTime;

    switch (artText.state) {
    case 1: // 进入动画
        artText.alpha = artText.time / artTextEnterTime;
        artText.scale = 2.5f - (artText.time / artTextEnterTime) * 0.5f; // 从2.5缩小到2.0

        if (artText.time >= artTextEnterTime) {
            artText.state = 2; // 显示状态
            artText.time = 0.0f;
            artText.alpha = 1.0f;
            artText.scale = 2.0f;
        }
        break;

    case 2: // 显示状态
        if (artText.time >= artTextDisplayTime) {
            artText.state = 3; // 退出动画
            artText.time = 0.0f;
        }
        break;

    case 3: // 退出动画
        artText.alpha = 1.0f - (artText.time / artTextExitTime);
        artText.scale = 2.0f - (artText.time / artTextExitTime) * 1.0f; // 从2.0缩小到1.0

        if (artText.time >= artTextExitTime) {
            artText.state = 0; // 隐藏
            artText.active = false;
            artText.alpha = 0.0f;
            artText.scale = 2.0f;
        }
        break;
    }
}

void UIManager::RenderArtText(const ArtTextInfo& artText) {
    if (!artText.active || artText.alpha <= 0.0f) return;
    if (!textRenderer) return;

    // 计算文本宽度以居中显示
    float textWidth = textRenderer->CalculateTextWidth(artText.text, artText.scale);
    float x = artText.x - textWidth / 2.0f;
    float y = artText.y;

    // 设置颜色（包含透明度）
    glm::vec4 color = artText.color;
    color.a = artText.alpha;

    // 渲染主文本
    textRenderer->RenderTextWithAlpha(artText.text, x, y, artText.scale, color);

    // 添加简单的阴影效果
    if (artText.alpha > 0.3f) {
        glm::vec4 shadowColor(0.0f, 0.0f, 0.0f, color.a * 0.5f);
        textRenderer->RenderTextWithAlpha(artText.text, x + 3.0f, y + 3.0f, artText.scale, shadowColor);
    }
}
