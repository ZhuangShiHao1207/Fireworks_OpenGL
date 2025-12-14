// include/UIManager.h
#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "TextRenderer.h"

// UI元素基类
class UIElement {
public:
    enum ElementType {
        TEXT,
        BUTTON,
        PANEL,
        PROGRESS_BAR
    };

    UIElement(ElementType type) : type(type), visible(true), active(true) {}
    virtual ~UIElement() = default;

    virtual void Render(TextRenderer* textRenderer) = 0;
    virtual void Update(float deltaTime) = 0;

    void SetPosition(float x, float y) { this->x = x; this->y = y; }
    void SetSize(float width, float height) { this->width = width; this->height = height; }
    void SetVisible(bool visible) { this->visible = visible; }
    void SetActive(bool active) { this->active = active; }

    bool Contains(float testX, float testY) const {
        return testX >= x && testX <= x + width && testY >= y && testY <= y + height;
    }

    ElementType GetType() const { return type; }
    bool IsVisible() const { return visible; }
    bool IsActive() const { return active; }

protected:
    ElementType type;
    float x = 0, y = 0;
    float width = 0, height = 0;
    bool visible;
    bool active;
};

// 文本元素
class TextElement : public UIElement {
public:
    TextElement() : UIElement(TEXT) {}

    void SetText(const std::string& text) { this->text = text; }
    void SetColor(const glm::vec4& color) { this->color = color; }
    void SetScale(float scale) { this->scale = scale; }
    void SetFontSize(float fontSize) { this->fontSize = fontSize; }

    void Render(TextRenderer* textRenderer) override;
    void Update(float deltaTime) override;

    const std::string& GetText() const { return text; }
    const glm::vec4& GetColor() const { return color; }  // 添加GetColor方法

private:
    std::string text;
    glm::vec4 color = glm::vec4(1.0f);
    float scale = 1.0f;
    float fontSize = 24.0f;
};

// 按钮元素（可以扩展）
class ButtonElement : public UIElement {
public:
    enum ButtonState {
        NORMAL,
        HOVER,
        PRESSED,
        DISABLED
    };

    ButtonElement() : UIElement(BUTTON), state(NORMAL) {}

    void SetText(const std::string& text) { this->text = text; }
    void SetTextColor(const glm::vec4& color) { this->textColor = color; }
    void SetBackgroundColor(const glm::vec4& color) { this->backgroundColor = color; }
    void SetHoverColor(const glm::vec4& color) { this->hoverColor = color; }

    void Render(TextRenderer* textRenderer) override;
    void Update(float deltaTime) override;

    void SetState(ButtonState newState) { state = newState; }

private:
    std::string text;
    glm::vec4 textColor = glm::vec4(1.0f);
    glm::vec4 backgroundColor = glm::vec4(0.3f, 0.3f, 0.3f, 0.7f);
    glm::vec4 hoverColor = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
    ButtonState state;
};

// UI管理器
class UIManager {
public:
    UIManager();
    ~UIManager();

    // 初始化
    bool Initialize(unsigned int screenWidth, unsigned int screenHeight);

    // 清理资源
    void Cleanup();  // 添加Cleanup声明

    // 渲染所有UI元素
    void Render(float deltaTime);

    // 更新窗口大小
    void UpdateScreenSize(unsigned int width, unsigned int height);

    // UI元素管理
    void AddElement(const std::string& id, UIElement* element);
    void RemoveElement(const std::string& id);
    UIElement* GetElement(const std::string& id);
    TextElement* GetTextElement(const std::string& id);
    // 艺术字动画控制
    void StartArtTextAnimation(int index);  // F1-F5对应index 0-4

    // 创建预设UI元素
    void CreateDefaultUI();

    // UI状态控制
    void ShowHint(const std::string& text, float duration = 2.0f);
    void IncrementFireworkCount();  // 添加此方法声明
    void SetFireworkCount(int count);
    void SetFireworkType(int type);
    void SetFPS(float fps);
    void SetMouseEnabled(bool enabled);
    void SetSceneLightsEnabled(bool enabled);
    void SetAutoTestMode(bool enabled);
    void ToggleAllText();

    // 鼠标交互
    void OnMouseMove(float x, float y);
    void OnMouseClick(float x, float y);

    // 获取屏幕尺寸
    unsigned int GetScreenWidth() const { return screenWidth; }
    unsigned int GetScreenHeight() const { return screenHeight; }

private:
    // 内部方法
    bool LoadFonts();
    void RenderHint(float deltaTime);

    // UI系统组件
    TextRenderer* textRenderer;
    std::map<std::string, UIElement*> uiElements;  // uiElements成员

    // 屏幕尺寸
    unsigned int screenWidth, screenHeight;

    // 临时提示系统
    struct HintInfo {
        std::string text;
        float duration;
        float timeLeft;
        float alpha;
    } currentHint;

    // UI状态
    bool showControlHints = true;
    float uiFadeTime = 0.0f;

    // 内部状态
    int fireworkCount = 0;

    // 艺术字系统
    struct ArtTextInfo {
        std::string text;
        glm::vec4 color;
        float x, y;
        float scale;
        float alpha;
        float time;
        int state;  // 0:隐藏, 1:进入, 2:显示, 3:退出
        bool active;
    };

    std::vector<ArtTextInfo> artTexts;

    // 艺术字动画参数
    float artTextEnterTime = 1.0f;
    float artTextDisplayTime = 3.0f;
    float artTextExitTime = 1.0f;

    // 艺术字相关方法
    void InitializeArtTexts();
    void UpdateArtText(ArtTextInfo& artText, float deltaTime);
    void RenderArtText(const ArtTextInfo& artText);

    //比例缩放相关
    float CalculateUIScale() const;
    void UpdateSimpleUIPositions();

};