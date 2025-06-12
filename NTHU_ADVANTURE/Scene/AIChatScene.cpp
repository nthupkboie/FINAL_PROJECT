#include "AIChatScene.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Resources.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>

AIChatScene::AIChatScene() 
    : isActive(true),
      currentInput(""),
      font(nullptr),
      boxWidth(800.0f),
      boxHeight(600.0f),
      boxX(240.0f),
      boxY(100.0f),
      padding(20.0f) {
    textColor = al_map_rgb(255, 255, 255);  // 白色文字
    boxColor = al_map_rgba(0, 0, 0, 200);   // 半透明黑色背景
}

void AIChatScene::Initialize() {
    // 方法1: 直接載入字體 (不使用 Resources)
    font = al_load_ttf_font("resources/fonts/normal.ttf", 24, 0);
    
    // 方法2: 如果 Resources 返回的是 ALLEGRO_FONT* 而不是 shared_ptr
    // font = Engine::Resources::GetInstance().GetFont("normal.ttf", 24);
    
    // 如果兩種方法都失敗，使用內建字體
    if (!font) {
        font = al_create_builtin_font();
    }
    
    // 初始消息
    AddMessage("AI: 你好！我是AI助手，請問有什麼可以幫你的嗎？");
}

void AIChatScene::Terminate() {
    // 只釋放我們直接載入的字體
    if (font && font != al_create_builtin_font()) {
        // 檢查是否是我們直接載入的字體
        // 如果是通過 Resources 獲取的，不要釋放
        // 這裡假設我們使用方法1直接載入
        al_destroy_font(font);
    }
    IScene::Terminate();
}

// 其他成員函數保持不變...
void AIChatScene::Update(float deltaTime) {
    // 簡單場景不需要每幀更新
}

void AIChatScene::Draw() const {
    // 繪製對話框背景
    al_draw_filled_rectangle(boxX, boxY, 
                            boxX + boxWidth, boxY + boxHeight, 
                            boxColor);
    al_draw_rectangle(boxX, boxY, 
                     boxX + boxWidth, boxY + boxHeight, 
                     al_map_rgb(255,255,255), 2);
    
    // 繪製聊天記錄
    float currentY = boxY + padding;
    for (const auto& msg : messages) {
        al_draw_text(font, textColor, boxX + padding, currentY, ALLEGRO_ALIGN_LEFT, msg.c_str());
        currentY += al_get_font_line_height(font) + 5;
    }
    
    // 繪製輸入框
    al_draw_filled_rectangle(boxX, boxY + boxHeight - 60, 
                            boxX + boxWidth, boxY + boxHeight - 20, 
                            al_map_rgba(64, 64, 64, 200));
    al_draw_text(font, textColor, boxX + padding, boxY + boxHeight - 50, 
                ALLEGRO_ALIGN_LEFT, ("You: " + currentInput).c_str());
    
    // 繪製返回提示
    al_draw_text(font, textColor, boxX + boxWidth - 150, boxY + boxHeight - 50, 
                ALLEGRO_ALIGN_LEFT, "Press ESC to back");
}

void AIChatScene::OnKeyDown(int keyCode) {
    // 處理鍵盤輸入
    if (keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z) {
        currentInput += 'a' + (keyCode - ALLEGRO_KEY_A);
    } 
    else if (keyCode == ALLEGRO_KEY_SPACE) {
        currentInput += " ";
    } 
    else if (keyCode == ALLEGRO_KEY_BACKSPACE && !currentInput.empty()) {
        currentInput.pop_back();
    } 
    else if (keyCode == ALLEGRO_KEY_ENTER && !currentInput.empty()) {
        SendMessage();
    }
    else if (keyCode == ALLEGRO_KEY_ESCAPE) {
        BackToStart();
    }
}

void AIChatScene::AddMessage(const std::string& message) {
    messages.push_back(message);
    
    // 限制消息數量
    if (messages.size() > 15) {
        messages.erase(messages.begin());
    }
}

void AIChatScene::SendMessage() {
    if (!currentInput.empty()) {
        AddMessage("You: " + currentInput);
        
        // 簡單模擬AI回應
        AddMessage("AI: 我收到了你的消息: \"" + currentInput + "\"");
        
        currentInput.clear();
    }
}

void AIChatScene::BackToStart() {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}